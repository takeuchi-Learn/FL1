#include "GameCamera.h"

#include"../Engine/Input/Input.h"

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
	if(angle <= 0)
	{
		angle = 0.f;
		addStartTimer = true;
	}
	else
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
		const float objAngleZ = obj->getRotation().z;
		
		// angleの最低値
		constexpr float angleMin = 1.0f;
		if(angle <= angleMin && !startLerp)
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
	if(startTimer >= startTimeMax)
	{
		changeCameraState();
	}
}

void GameCamera::changeCameraState()
{
	switch (cameraState)
	{
	case GameCamera::CameraState::START:
		cameraState = CameraState::INPUT;
		break;
	case GameCamera::CameraState::INPUT:
		break;
	case GameCamera::CameraState::OTHER:
		break;
	default:
		break;
	}
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
	if (Input::getInstance()->hitKey(DIK_LEFT))
	{
		angle -= frameAngle;
	} else
		if (Input::getInstance()->hitKey(DIK_RIGHT))
		{
			angle += frameAngle;
		}
}

#pragma endregion


void GameCamera::angleToUp(float angle, DirectX::XMFLOAT2& upXY)
{
	// 0.0fで上を向くように90.0fを加算
	angle += 90.0f;

	// 変換
	constexpr float pi = 3.14f;
	upXY.x = std::cos(angle * pi / 180.0f);
	upXY.y = std::sin(angle * pi / 180.0f);
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

void GameCamera::followObject()
{
	if (!obj)return;

	// 追従
	// Targetをobjの座標に
	DirectX::XMFLOAT3 objPos = obj->getPosition();
	setTarget(objPos);

	// eyeをXYだけobjの座標に
	DirectX::XMFLOAT3 eye = objPos;
	eye.z = getEye().z;
	setEye(eye);
}

GameCamera::GameCamera(AbstractGameObj* obj)
	:Camera(WinAPI::window_width,
			WinAPI::window_height)
	, obj(obj)
{
	// 平行投影の場合、相当カメラ離したほうがいい(デフォルト猿モデルだとeyeのZ値-500くらいがベスト)

	// 平行投影に変更
	setPerspectiveProjFlag(false);
}

void GameCamera::gameCameraUpdate()
{
	
	switch (cameraState)
	{
	case GameCamera::CameraState::START:
		updateStart();
		break;
	case GameCamera::CameraState::INPUT:
		updateInput();
		break;
	default:
		break;
	}

	// 追従
	followObject();
}
