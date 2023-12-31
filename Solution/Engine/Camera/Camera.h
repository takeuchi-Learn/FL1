﻿#pragma once

#include <DirectXMath.h>

#include <System/DX12Base.h>

class Camera
{
protected:
	// エイリアス
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;

	static DX12Base* dxBase;

private:
	XMVECTOR cameraAxisX{};
	XMVECTOR cameraAxisY{};
	XMVECTOR cameraAxisZ{};
	XMMATRIX matViewPort{};
	// ビュー行列
	XMMATRIX matView = DirectX::XMMatrixIdentity();
	// ビルボード行列
	XMMATRIX matBillboard = DirectX::XMMatrixIdentity();
	// Y軸回りビルボード行列
	XMMATRIX matBillboardY = DirectX::XMMatrixIdentity();
	// 射影行列
	XMMATRIX matProjection = DirectX::XMMatrixIdentity();
	// ビュー射影行列
	XMMATRIX matViewProjection = DirectX::XMMatrixIdentity();
	// VPV行列
	XMMATRIX matVPV = DirectX::XMMatrixIdentity();
	// ビュー行列ダーティフラグ
	bool viewDirty = false;
	// 射影行列ダーティフラグ
	bool projectionDirty = false;
	// 視点座標
	XMFLOAT3 eye = { 0, 0, -20 };
	// 注視点座標
	XMFLOAT3 target = { 0, 0, 0 };
	// 上方向ベクトル
	XMFLOAT3 up = { 0, 1, 0 };
	// アスペクト比
	float aspectRatio = 1.0f;

	float nearZ;
	float farZ;
	float fogAngleYRad;

	// 透視投影かどうか
	bool perspectiveProjFlag = true;

	// --------------------
	// メンバ関数
	// --------------------
private:

	// ビュー行列を更新
	void updateViewMatrix();

	// 射影行列を更新
	void updateProjectionMatrix();

	void updateMatrix();

	virtual void preUpdate() {};
	virtual void postUpdate() {};

public:
	inline bool getPerspectiveProjFlag() const { return perspectiveProjFlag; }

	/// @brief 視線ベクトル（奥方向）を取得
	inline const XMVECTOR& getEyeVec() const { return cameraAxisZ; }

	/// @brief 右方向を取得
	inline const XMVECTOR& getRightVec() const { return cameraAxisX; }

	/// @brief 上方向を取得
	inline const XMVECTOR& getUpVec() const { return cameraAxisY; }

	/// @brief ビューポート行列を取得
	inline const XMMATRIX& getViewPortMatrix() const { return matViewPort; }

	// ビュー行列の取得
	// @return matView
	inline const XMMATRIX& getViewMatrix() const { return matView; }

	// 射影行列の取得
	// @return matProjection
	inline const XMMATRIX& getProjectionMatrix() const { return matProjection; }

	// ビュー射影行列の取得
	inline const XMMATRIX& getViewProjectionMatrix() const { return matViewProjection; }

	// ビルボード行列の取得
	inline const XMMATRIX& getBillboardMatrix() const { return matBillboard; }

	// Y軸固定ビルボード行列の取得
	inline const XMMATRIX& getBillboardMatrixY() const { return matBillboardY; }

	// 視点座標の取得
	inline const XMFLOAT3& getEye() const { return eye; }

	// 視点座標の設定
	inline void setEye(const XMFLOAT3& eye) { this->eye = eye; viewDirty = true; }

	// 注視点座標の取得
	inline const XMFLOAT3& getTarget() const { return target; }

	/// @brief 透視投影かどうかを設定
	inline void setPerspectiveProjFlag(bool perspectiveProjFlag) { this->perspectiveProjFlag = perspectiveProjFlag; projectionDirty = true; };

	// 注視点座標の設定
	inline void setTarget(const XMFLOAT3& target) { this->target = target; viewDirty = true; }

	// 上方向ベクトルの取得
	inline const XMFLOAT3& getUp() const { return up; }
	// 上方向ベクトルの設定
	inline void setUp(const XMFLOAT3& up) { this->up = up; viewDirty = true; }

	inline void setNearZ(const float nearZ) { this->nearZ = nearZ; projectionDirty = true; }
	inline void setFarZ(const float farZ) { this->farZ = farZ; projectionDirty = true; }

	inline void setFogAngleYRad(const float fogAngleYRad) { this->fogAngleYRad = fogAngleYRad; projectionDirty = true; }

	inline float getNearZ() const { return nearZ; }
	inline float getFarZ() const { return farZ; }

	inline float getFogAngleYRad() const { return fogAngleYRad; }

	XMFLOAT3 screenPos2WorldPos(const XMFLOAT3& screenPos) const;
	XMVECTOR screenPos2WorldPosVec(const XMFLOAT3& screenPos) const;

	inline void screenPos2WorldPos(const XMFLOAT3& screenPos, XMFLOAT3& outBuf) const
	{
		DirectX::XMStoreFloat3(&outBuf, screenPos2WorldPosVec(screenPos));
	}

	inline XMFLOAT3 calcLookVec() const
	{
		return XMFLOAT3(target.x - eye.x,
						target.y - eye.y,
						target.z - eye.z);
	}
	XMFLOAT3 calcLookNormal() const;

	/// @brief カメラを回転
	/// @param targetlength カメラから注視点までの距離
	/// @param angleX X軸周りの回転角(-PI/2 ~ PI/2の範囲で送る)
	/// @param angleY Y軸周りの回転角(0 ~ 2PIの範囲で送る)
	void rotation(const float targetlength,
				  const float angleX, const float angleY);

	void moveForward(const float speed);

	void moveRight(const float speed);

	/// @brief ベクトルによるカメラ移動(eyeのみの移動、targetは変わらない)
	/// @param move 移動量
	void moveEye(const XMFLOAT3& move);
	void moveEye(const XMVECTOR& move);

	/// @brief ベクトルによる移動(eyeとtargetを移動)
	/// @param 移動量
	void moveCamera(const XMFLOAT3& move);
	void moveCamera(const XMVECTOR& move);

	inline const XMMATRIX& getMatVPV() const { return matVPV; }

	Camera(const float window_width, const float window_height);

	inline Camera(POINT windowSize) :Camera((float)windowSize.x, (float)windowSize.y) {}

	~Camera();

	void update();
};
