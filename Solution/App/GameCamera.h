#pragma once
#include "../Engine/Camera/CameraObj.h"

// 後々の追従を想定してObjのやつ

class GameCamera :
    public CameraObj
{
private:
	// 角度Z
	float angle = 0.0f;

private:
	void angleToUp(float angle,DirectX::XMFLOAT2& float2);
	void checkInput();

public:

	/// @brief コンストラクタ
	/// @param obj プレイヤーのポインタ(追従させるために渡す)
	GameCamera(AbstractGameObj* obj = nullptr);

	/// @brief 更新
	void gameCameraUpdate();

};

