#include "GameCamera.h"

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
	constexpr float maxAngle = 180.0f;

#pragma region ジャイロ

	getAccelX = sensor->GetAccelX();
	getAccelZ = sensor->GetAccelZ();
	getGyroX = sensor->GetGyroX();
	getGyroZ = sensor->GetGyroZ();


	// 入力確認してカメラを傾ける
	angle = atan2f(getAccelX, getAccelZ) * 58.0f;

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

void GameCamera::gameCameraUpdate(Sensor* sensor)
{
	this->sensor = sensor;

	// 回転
	upRotate();

	// 追従
	followObject();

}
