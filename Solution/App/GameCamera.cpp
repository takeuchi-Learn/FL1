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
	if (angleDeg <= 0)
	{
		angleDeg = 0.f;
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
		angleDeg -= startFrameAngle;
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
	rotation();

	// 上ベクトル回転
	upRotate();
}

void GameCamera::rotation()
{
	// 指定の入力で角度リセット
	{
		constexpr BYTE key = DIK_R;
		constexpr WORD pad_xinput = XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y;
		constexpr int pad_other = JSMASK_W | JSMASK_N;

		const bool padInput = PadImu::ins()->getDevCount() > 0 && PadImu::ins()->hitButtons(PadImu::ins()->getStates()[0].buttons, pad_other);

		if (Input::ins()->hitKey(key) ||
			Input::ins()->hitPadButton(pad_xinput) ||
			padInput)
		{
			angleDeg = 0.f;
		}
	}

	imuInputRotation();
}

void GameCamera::imuInputRotation()
{
	sensor->update();
	// 加速度取得
	accel.right = -sensor->GetAccelX();
	accel.up = -sensor->GetAccelZ();
	accel.forward = -sensor->GetAccelY();
	// 角速度取得
	gyro.pitch = -sensor->GetGyroX();
	gyro.yaw = -sensor->GetGyroZ();
	gyro.roll = -sensor->GetGyroY();

	const bool imuPadIsConnected = PadImu::ins()->getDevCount() > 0;

	if (imuPadIsConnected)
	{
		const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);

		// 加速度取得
		accel.right = -state.accelX;
		accel.up = -state.accelY;
		accel.forward = state.accelZ;
		// 角速度取得
		gyro.pitch = -state.gyroX;
		gyro.yaw = -state.gyroY;
		gyro.roll = -state.gyroZ;
	}
	// 単位がDegree/Secなので、1フレームでの値に変換する
	gyro.pitch /= DX12Base::ins()->getFPS();
	gyro.yaw /= DX12Base::ins()->getFPS();
	gyro.roll /= DX12Base::ins()->getFPS();

	// 相補フィルターで補正
	{
		// 調整項目
		const float invRaito = 1.f - angleFilterRaito;
		const float rollAccel = XMConvertToDegrees(std::atan2(accel.right, -accel.up));

		angleDeg = angleFilterRaito * (angleDeg + gyro.roll) + invRaito * rollAccel;
	}

	// 静止状態を大きめに取る
	if (angleDeg <= angleStopRange && -angleStopRange <= angleDeg)
	{
		angleDeg = 0.0f;
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
	angleDeg = std::clamp(angleDeg, -maxAngle, maxAngle);

	// angleを上ベクトルに変換してセット
	DirectX::XMFLOAT2 upXY{};
	angleToUp(angleDeg, upXY);
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

void GameCamera::IMUDelete()
{}

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