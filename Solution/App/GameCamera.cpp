#include "GameCamera.h"

#include"../Engine/Input/Input.h"

void GameCamera::angleToUp(float angle, DirectX::XMFLOAT2& upXY)
{
	// 0.0fで上を向くように90.0fを加算
	angle += 90.0f;

	constexpr float pi = 3.141592f;
	upXY.x = std::cos(angle * pi / 180.0f);
	upXY.y = std::sin(angle * pi / 180.0f);
}
 
void GameCamera::upRotate()
{
	// 傾きの最大値
	constexpr float maxAngle = 60.0f;
	// 1フレームの回転量
	constexpr float frameAngle = 3.0f;

	// 入力確認してカメラを傾ける
	// 上ベクトル変更する仕様っぽい
	if(Input::getInstance()->hitKey(DIK_LEFT) && angle >= -maxAngle)
	{
		angle -= frameAngle;
	}
	else
	if (Input::getInstance()->hitKey(DIK_RIGHT) && angle <= maxAngle)
	{
		angle += frameAngle;
	}

	// 制限
	if (angle >= maxAngle)angle = maxAngle;
	else if (angle <= -maxAngle)angle = -maxAngle;

	// 上ベクトルをセット
	DirectX::XMFLOAT2 upXY;
	angleToUp(angle, upXY);
	setUp(DirectX::XMFLOAT3(upXY.x, upXY.y, 0));
}

void GameCamera::movePosition()
{
	DirectX::XMFLOAT3 xyPos( 0,0,0 );
	constexpr float frameMove = 0.4f;
	if (Input::getInstance()->hitKey(DIK_W))
	{
		xyPos.y += frameMove;
	}
	if (Input::getInstance()->hitKey(DIK_S))
	{
		xyPos.y -= frameMove;
	}
	if (Input::getInstance()->hitKey(DIK_A))
	{
		xyPos.x -= frameMove;
	}
	if (Input::getInstance()->hitKey(DIK_D))
	{
		xyPos.x += frameMove;
	}
	if (Input::getInstance()->hitKey(DIK_Q))
	{
		xyPos.z -= frameMove;
	}
	if (Input::getInstance()->hitKey(DIK_E))
	{
		xyPos.z += frameMove;
	}
	moveCamera(xyPos);


}

GameCamera::GameCamera(AbstractGameObj* obj) : CameraObj(obj)
{
	// 平行投影の場合、相当カメラ離したほうがいい(-150くらいがベストs)

	// 平行投影に変更
	setPerspectiveProjFlag(false);
}

void GameCamera::gameCameraUpdate()
{
	upRotate();

#ifdef _DEBUG

	// デバッグ用
	// カメラ移動
	movePosition();

	// 透視投影と平行投影の切り替え
	if (Input::getInstance()->triggerKey(DIK_Z))
	{
		setPerspectiveProjFlag(!getPerspectiveProjFlag());
	}
#endif // _DEBUG

}
