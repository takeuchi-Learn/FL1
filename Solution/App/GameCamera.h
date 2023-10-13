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
	/// @brief 角度を上ベクトルに変換
	/// @param angle 角度
	/// @param float2 上ベクトルを格納するXMFLOAT2
	void angleToUp(float angle, DirectX::XMFLOAT2& float2);
	
	/// @brief 上ベクトルの回転(カメラ回転)
	void upRotate();

public:

	/// @brief コンストラクタ
	/// @param obj プレイヤーのポインタ(追従させるために渡す)
	GameCamera(AbstractGameObj* obj = nullptr);

	/// @brief 更新(元々のupdateと被らないように名前長くしてる)
	void gameCameraUpdate();

	/// @brief ジャイロの値のセット。
	void setGyroValue(float value) { angle = value; }
};

