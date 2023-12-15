#include "GameCamera.h"
#include <algorithm>
#include <Input/Input.h>
#include <Input/PadImu.h>
#include <Util/YamlLoader.h>

using namespace DirectX;

namespace
{
	constexpr const char yamlPath[] = "Resources/DataFile/camera.yml";
}

GameCamera::GameCamera(BillboardData* obj)
	:Camera(WinAPI::window_width,
			WinAPI::window_height)
	, obj(obj)
{
	// 平行投影に変更
	setPerspectiveProjFlag(false);

	loadYaml();

	// センサーの生成
	if (sensor == nullptr)
	{
		sensor = Sensor::ins();
	}
}

bool GameCamera::loadYaml()
{
	Yaml::Node node{};
	if (YamlLoader::LoadYamlFile(node, yamlPath))
	{
		return true;
	}

	try
	{
		LoadYamlData(node, angleFilterRaito);
		LoadYamlData(node, angleStopRange);
		LoadYamlData(node, stickAngleShiftVal);
		const auto str = node["isActiveStickControll"].As<std::string>("false");
		if (str == "true" || str == "TRUE")
		{
			isActiveStickControll = true;
		} else
		{
			isActiveStickControll = false;
		}
	} catch (...)
	{
		return true;
	}

	if (YamlLoader::LoadYamlFile(node, "Resources/DataFile/stickRange.yml"))
	{
		return true;
	}

	try
	{
		auto& root = node["stickRange"];
		auto& L = root["L"];
		auto& R = root["R"];

		XMFLOAT2 min{}, max{};

		LoadYamlDataToFloat2(L, min);
		LoadYamlDataToFloat2(L, max);
		stickRangeL.min = min;
		stickRangeL.max = max;

		LoadYamlDataToFloat2(R, min);
		LoadYamlDataToFloat2(R, max);
		stickRangeR.min = min;
		stickRangeR.max = max;
	} catch (...)
	{
		return true;
	}

	return false;
}

#pragma region START

void GameCamera::updateStart()
{
	// 開始時の自動移動
	startAutoRot();
	// 上ベクトル回転
	upRotate();
	// スタートタイマー更新
	updateStartTimer();
}

void GameCamera::startAutoRot()
{
	// 開始時にプレイヤーのアングルが0の方が見た目いいかもしれない
	// 実装した
	// 最後にガクってなるのはカメラの傾きをやめたせい

	// 角度0になったら固定してタイマー加算開始
	if (angle <= 0)
	{
		angle = 0.f;
		addStartTimer = true;
	} else
	{
		// 傾きの最低値
		constexpr float startFrameAngleMin = 0.1f;
		// 最低値を下回ったら固定
		if (startFrameAngle <= startFrameAngleMin)
		{
			startFrameAngle = startFrameAngleMin;
		} else
		{
			// 1フレームに減算する量
			constexpr float subStartFrameAngle = 0.25f;
			// 段々遅くするために少しずつ減らす
			startFrameAngle -= subStartFrameAngle;
		}

		// 少しずつ平行にしていく
		angle -= startFrameAngle;
	}
}

void GameCamera::updateStartTimer()
{
	if (!addStartTimer) { return; }
	++startTimer;

	// タイマーの最大値
	// 自動傾き終了後にタイマーが起動し、以下の時間を上回ったら入力受付開始となる
	// todo シーン側で行うべき要素
	constexpr unsigned short startTimeMax = 0;
	if (startTimer >= startTimeMax)
	{
		cameraState = CameraState::INPUT;
	}
}

void GameCamera::preUpdate()
{
	gameCameraUpdate();
}

#pragma endregion

#pragma region INPUT

void GameCamera::updateInput()
{
	// 入力確認とそれに応じた傾き
	// ジャイロ導入でここの中身を書き換える
	checkInput();

	// 上ベクトル回転
	upRotate();
}

void GameCamera::checkInput()
{
	// 指定の入力で角度リセット
	{
		constexpr BYTE key = DIK_R;
		constexpr WORD pad_xinput = XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y;
		constexpr int pad_other = JSMASK_W | JSMASK_N;

		const bool padInput = PadImu::ins()->getDevCount() > 0 && PadImu::ins()->hitButtons(PadImu::ins()->getStates()[0].buttons, pad_other);

		if (Input::ins()->hitKey(key) ||
			Input::ins()->hitPadButton(pad_xinput) ||
			padInput || 
			sensor->GetButton())
		{
			prevAngle = 0.f;
			angle = 0.f;
			gyro_angle = 0.f;
		}
	}

	// 使いたいやつに応じてコメントアウトしたり解除したりしてください

	checkKeyInput();
	checkSensorInput();
}

void GameCamera::checkSensorInput()
{
	sensor->update();
	// 加速度取得
	accel.x = sensor->GetAccelX();
	accel.y = sensor->GetAccelY();
	accel.z = sensor->GetAccelZ();
	// 角速度取得
	gyro.x = sensor->GetGyroX();
	gyro.y = -sensor->GetGyroY();
	gyro.z = sensor->GetGyroZ();

	float fps = DX12Base::ins()->getFPS();

	if (!sensor->GetButton())
	{
		// ジャイロのYの値のズレ修正用
		float offsetY = 0.15f;
		//加速度センサーから角度を算出
		float acc_angle = XMConvertToDegrees(atan2(accel.x, accel.z + abs(accel.y)));
		//数値積分
		gyro_angle += (gyro.y - offsetY) * (fps * 0.001f);

		// 相補フィルターで補正
		angle = (0.8f * gyro_angle) + (0.2f * acc_angle);
	}

	const bool imuPadIsConnected = PadImu::ins()->getDevCount() > 0;

	if (imuPadIsConnected)
	{
		const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);

		// 加速度取得
		accel.x = -state.accelX / 16384.f;
		accel.y = state.accelY / 16384.f;
		accel.z = state.accelZ / 16384.f;
		// 角速度取得
		gyro.x = state.gyroX / 131.f;
		gyro.y = state.gyroY / 131.f;
		gyro.z = -state.gyroZ / 131.f;

		// 相補フィルターで補正
		angle += angleFilterRaito * (accel.x / DX12Base::ins()->getFPS())
			+ (1.f - angleFilterRaito) * std::atan2(accel.x, accel.y) + gyro.z;
	}

	if (imuPadIsConnected)
	{
		const auto& state = PadImu::ins()->getStates()[0];
		const auto& preState = PadImu::ins()->getPreStates()[0];

		if (isActiveStickControll)
		{
			float stick = state.stickRX;
			auto* range = &stickRangeR;

			if (JS_TYPE_JOYCON_LEFT
			   == PadImu::ins()->getContollerType(0))
			{
				stick = state.stickLX;
				range = &stickRangeL;
			}

			stick -= std::lerp(range->min.x, range->max.x, 0.5f);

			angle += stick * stickAngleShiftVal;
		}
	}

	// 静止状態を大きめに取る
	if (angle <= angleStopRange && -angleStopRange <= angle)
	{
		angle = 0.0f;
	}

	// 前の角度を保存
	prevAngle = angle;
	prevGyroY = gyro.y;
}

// todo 関数名とやっていることが違う（checkではなく角度変更をしている）
void GameCamera::checkKeyInput()
{
	// 1フレームの回転量(多分ジャイロから受け取るようにしたら消す)
	constexpr float frameAngle = 3.0f;

	// 入力確認してカメラを傾ける
	// ジャイロ使用時は直接角度を代入するためこちらも消す
	if (Input::ins()->hitKey(DIK_LEFT) ||
		Input::ins()->hitPadButton(XINPUT_GAMEPAD_DPAD_LEFT))
	{
		angle -= frameAngle;
	} else if (Input::ins()->hitKey(DIK_RIGHT) ||
			   Input::ins()->hitPadButton(XINPUT_GAMEPAD_DPAD_RIGHT))
	{
		angle += frameAngle;
	}
}

#pragma endregion

#pragma region CLEAR
void GameCamera::updateClear()
{}
#pragma endregion

void GameCamera::angleToUp(float angle, DirectX::XMFLOAT2& upXY)
{
	// 0.0fで上を向くように90.0fを加算
	angle += 90.0f;

	// 変換
	upXY.x = std::cos(XMConvertToRadians(angle));
	upXY.y = std::sin(XMConvertToRadians(angle));
}

void GameCamera::upRotate()
{
	// 傾きの最大値
	constexpr float maxAngle = 35.0f;

	// 制限
	angle = std::clamp(angle, -maxAngle, maxAngle);

	// angleを上ベクトルに変換してセット
	DirectX::XMFLOAT2 upXY{};
	angleToUp(angle, upXY);
	setUp(DirectX::XMFLOAT3(upXY.x, upXY.y, 0));
}

void GameCamera::followObject(const bool followX)
{
	if (!obj) { return; }

	// 追従
	// Targetをobjの座標に
	DirectX::XMFLOAT3 objPos = obj->position;

	if (!followX)
	{
		objPos.x = getEye().x;
	}

	setTarget(objPos);

	// eyeをXYだけobjの座標に
	DirectX::XMFLOAT3 eye = objPos;
	eye.z = getEye().z;

	if (!followX)
	{
		objPos.x = getTarget().x;
	}
	setEye(eye);
}

void GameCamera::setFollowFlag(const bool flag)
{
	if (cameraState == CameraState::INPUT || cameraState == CameraState::FOLLOW_OFF)
	{
		cameraState = flag ? CameraState::INPUT : CameraState::FOLLOW_OFF;
	}
}

void GameCamera::gameCameraUpdate()
{
	// todo カメラがどう動くかは、カメラの構成要素ではないので、別クラスに分けるべき。（ビヘイビアツリーやstateパターンを使う）

	switch (cameraState)
	{
	case GameCamera::CameraState::START:
		updateStart();
		break;
	case GameCamera::CameraState::INPUT:
	case GameCamera::CameraState::FOLLOW_OFF:
		updateInput();
		break;
	case GameCamera::CameraState::CLEAR:
		updateClear();
		break;
	}

	// 追従
	if (cameraState == GameCamera::CameraState::FOLLOW_OFF)
	{
		// Xは固定
		followObject(false);
	} else if (cameraState == GameCamera::CameraState::OTHER)
	{
	} else
	{
		// 追従
		followObject(true);
	}
}