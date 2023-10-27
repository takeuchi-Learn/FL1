#pragma once
#include "../Engine/Camera/Camera.h"
#include "../Engine/GameObject/AbstractGameObj.h"
#include <Imu/Sensor.h>

// 後々の追従を想定してObjのやつ
// CameraObjだと上ベクトルの制御が不可能になるからこちらで追従機能を追加したほうがいいかも

// Z軸回転と追従機能だけ持ったカメラクラス
class GameCamera :
	public Camera
{
private:
	// 追従Obj
	AbstractGameObj* obj = nullptr;
	// センサー
	Sensor* sensor = nullptr;
	// 角度Z
	float angle = 0.f;
	float getGyroX = 0.0f;

private:
	/// @brief 角度を上ベクトルに変換
	/// @param angle 角度
	/// @param float2 上ベクトルを格納するXMFLOAT2
	void angleToUp(float angle, DirectX::XMFLOAT2& float2);

	/// @brief 上ベクトルの回転(カメラ回転)
	void upRotate();

	/// @brief 追従
	void followObject();

public:

	/// @brief コンストラクタ
	/// @param obj プレイヤーのポインタ(追従させるために渡す)
	GameCamera(AbstractGameObj* obj = nullptr);

	/// @brief 更新(元々のupdateと被らないように名前長くしてる)
	void gameCameraUpdate(Sensor* sensor);

	/// @brief ジャイロの値のセット。
	void setGyroValue(float value) { angle = value; }

	/// @brief 追従先オブジェクト
	/// @param obj 
	void setParentObj(AbstractGameObj* obj) { this->obj = obj; }

	/// @brief Z座標
	/// @param z 
	void setEyeZ(float z) { setEye(DirectX::XMFLOAT3(0, 0, z)); }
};

