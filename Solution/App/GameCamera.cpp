#include "GameCamera.h"
#include <algorithm>
#include <Input/Input.h>
#include <Input/PadImu.h>
#include <Imu/Sensor.h>
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

void GameCamera::preUpdate()
{
	// todo カメラがどう動くかは、カメラの構成要素ではないので、別クラスに分けるべき。（ビヘイビアツリーやstateパターンを使う）

	switch (cameraState)
	{
	case GameCamera::CameraState::INPUT:
	case GameCamera::CameraState::FOLLOW_OFF:
		// 入力確認とそれに応じた傾き
		rotation();
		// 上ベクトル回転
		upRotate();
		break;
	}

	// 追従
	if (cameraState == GameCamera::CameraState::FOLLOW_OFF)
	{
		// Xは固定
		followObject(false);
	} else if (cameraState != GameCamera::CameraState::NO_UPDATE)
	{
		// 追従
		followObject(true);
	}
}

#pragma region INPUT

void GameCamera::rotation()
{
	// 入力無効なら何もしない
	if (!allowInput) { return; }

	// 指定の入力で角度リセット
	{
		constexpr BYTE key = DIK_R;
		constexpr WORD pad_xinput = XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y;
		constexpr int pad_other = JSMASK_W | JSMASK_N;

		const bool padInput = PadImu::ins()->getDevCount() > 0 && PadImu::ins()->hitButtons(PadImu::ins()->getStates()[0].buttons, pad_other);

		if (Input::ins()->hitKey(key) ||
			Input::ins()->hitPadButton(pad_xinput) ||
			padInput ||
			Sensor::ins()->CheckButton())
		{
			dInputAngleDeg -= angleDeg * 2.f;
			angleDeg = 0.f;
		}
	}

	directionalInputRotation();
	imuInputRotation();
}

void GameCamera::directionalInputRotation()
{
	constexpr float speed = 360.f;
	float val = 0.f;

	if (PadImu::ins()->getDevCount() > 0)
	{
		auto* range = &stickRangeR;
		const auto& state = PadImu::ins()->getStates().front();
		if (PadImu::ins()->getContollerType(0) == JS_TYPE_JOYCON_LEFT)
		{
			val = state.stickLX;
			stickRangeL;
		} else
		{
			val = state.stickRX;
		}

		// R = a + t * (b - a)
		// t = (R - a) / (b - a)
		val = (val - range->min.x) / (range->max.x - range->min.x);
		val = std::lerp(-1.f, 1.f, val);
	}

	if (Input::ins()->isVaildPadRStickX())
	{
		val = Input::ins()->hitPadRStickRaito().x;
	}

	if (Input::ins()->hitKey(DIK_RIGHT))
	{
		val += 1.f;
	}
	if (Input::ins()->hitKey(DIK_LEFT))
	{
		val -= 1.f;
	}

	dInputAngleDeg += speed * val / DX12Base::ins()->getFPS();
	angleDeg += dInputAngleDeg;
}

void GameCamera::imuInputRotation()
{
	auto* sensor = Sensor::ins();

	// 加速度取得
	accel.right = sensor->GetAccelX();
	accel.up = -sensor->GetAccelZ();
	accel.forward = sensor->GetAccelY();
	// 角速度取得
	gyro.pitch = sensor->GetGyroX();
	gyro.yaw = sensor->GetGyroZ();
	gyro.roll = sensor->GetGyroY();

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
