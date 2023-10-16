#include "GameCamera.h"

#include"../Engine/Input/Input.h"

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
	constexpr float maxAngle = 60.0f;

#pragma region ジャイロ導入で消す部分

	// 1フレームの回転量(多分ジャイロから受け取るようにしたら消す)
	constexpr float frameAngle = 3.0f;

	// 入力確認してカメラを傾ける
	// ジャイロ使用時は直接角度を代入するためこちらも消す
	if(Input::getInstance()->hitKey(DIK_LEFT) && angle >= -maxAngle)
	{
		angle -= frameAngle;
	}
	else
	if (Input::getInstance()->hitKey(DIK_RIGHT) && angle <= maxAngle)
	{
		angle += frameAngle;
	}
#pragma endregion

	// 制限
	if (angle >= maxAngle)angle = maxAngle;
	else if (angle <= -maxAngle)angle = -maxAngle;

	// angleを上ベクトルに変換してセット
	DirectX::XMFLOAT2 upXY;
	angleToUp(angle, upXY);
	setUp(DirectX::XMFLOAT3(upXY.x, upXY.y, 0));
}

GameCamera::GameCamera(AbstractGameObj* obj) : CameraObj(obj)
{
	// 平行投影の場合、相当カメラ離したほうがいい(デフォルト猿モデルだとeyeのZ値-500くらいがベスト)

	// 平行投影に変更
	setPerspectiveProjFlag(false);


	// プレイヤーとカメラの距離を700に設定
	setEye2TargetLen(700.f);
	// 横からの視点にしたいので勝手に角度変わらないようにする
	setRelativeRotaDeg(DirectX::XMFLOAT3(0, 0, 0));
}

void GameCamera::gameCameraUpdate()
{
	// 回転
	upRotate();
}
