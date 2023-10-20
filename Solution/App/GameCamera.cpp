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
	constexpr float maxAngle = 40.0f;

#pragma region ジャイロ導入で消す部分

	// 1フレームの回転量(多分ジャイロから受け取るようにしたら消す)
	constexpr float frameAngle = 3.0f;

	// 入力確認してカメラを傾ける
	// ジャイロ使用時は直接角度を代入するためこちらも消す
	if (Input::getInstance()->hitKey(DIK_LEFT) && angle >= -maxAngle)
	{
		angle -= frameAngle;
	} else
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
	// 回転
	upRotate();

	// 追従
	followObject();
	if (Input::getInstance()->hitKey(DIK_A))
	{
		DirectX::XMFLOAT3 eye = getEye();
		int fh3ifr = 0;
	}
}
