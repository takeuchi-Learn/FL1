﻿#include "GameCamera.h"
#include <Input/Input.h>

#include <PadImu.h>
#include <JoyShockLibrary.h>

using namespace DirectX;

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
		if (!startLerp)
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

		// 自動傾き終了時にプレイヤーのアングルが0になるように調整する
		// プレイヤーの角度を取得
		const float objAngleZ = obj->position.z;

		// angleの最低値
		constexpr float angleMin = 1.0f;
		if (angle <= angleMin && !startLerp)
		{
			angle = angleMin;
			startLerp = true;
		}

		if (startLerp && objAngleZ > -angleMin)
		{
			angle = 0.f;
			//obj->setRotationZ(0.f);
		}
	}
}

void GameCamera::updateStartTimer()
{
	if (!addStartTimer)return;
	startTimer++;

	// タイマーの最大値
	// 自動傾き終了後にタイマーが起動し、以下の時間を上回ったら入力受付開始となる
	constexpr unsigned short startTimeMax = static_cast<short>(60.f * 1.5f);
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

	// todo ジャイロ仮。加速度センサーのみで姿勢推定している。
	if (PadImu::ins()->getDevCount() > 0)
	{
		const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);

		// 補正の強度。値が大きいと誤差が吸収されるが反応が遅くなる
		constexpr float rate = 0.05f, invRate = 1.f - rate;

		const float roll = std::atan2(state.accelY, state.accelZ);
		const auto delta = DX12Base::ins()->getFPS() / 1000.f;

		angle = invRate * (angle + -state.accelX / delta) + rate * roll;
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
	constexpr float pi = 3.14f;
	upXY.x = std::cos(XMConvertToRadians(angle));
	upXY.y = std::sin(XMConvertToRadians(angle));
}

void GameCamera::upRotate()
{
	// 傾きの最大値
	constexpr float maxAngle = 40.0f;

	// 制限
	if (angle >= maxAngle)angle = maxAngle;
	else if (angle <= -maxAngle)angle = -maxAngle;

	// angleを上ベクトルに変換してセット
	DirectX::XMFLOAT2 upXY;
	angleToUp(angle, upXY);
	setUp(DirectX::XMFLOAT3(upXY.x, upXY.y, 0));
}

void GameCamera::followObject(const bool followX)
{
	if (!obj)return;

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

GameCamera::GameCamera(BillboardData* obj)
	:Camera(WinAPI::window_width,
			WinAPI::window_height)
	, obj(obj)
{

	// 平行投影に変更
	setPerspectiveProjFlag(false);
	//setEye(XMFLOAT3(0,0,-700));
}

void GameCamera::gameCameraUpdate()
{
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
	default:
		break;
	}

	// 追従
	if (cameraState != GameCamera::CameraState::OTHER &&
		cameraState != GameCamera::CameraState::FOLLOW_OFF)
	{
		// 追従
		followObject(true);
	} else
	{
		// Xは固定
		followObject(false);
	}
}