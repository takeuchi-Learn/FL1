/*****************************************************************//**
 * \file   CameraObj.h
 * \brief  追従機能とワールド行列を追加したカメラクラス
 *********************************************************************/

#pragma once

#include "GameObject/AbstractGameObj.h"
#include "Camera.h"
#include <memory>

 /// @brief 追従機能とワールド行列を追加したカメラクラス
class CameraObj
	: public Camera
{
private:
	AbstractGameObj* parentObj = nullptr;

	/// @brief 視点から注視点までの距離
	float eye2TargetLen = 30.f;

	/// @brief 親を基準とした回転
	DirectX::XMFLOAT3 relativeRotaDeg = XMFLOAT3(60, 0, 0);

	DirectX::XMFLOAT3 eye2TargetOffset = XMFLOAT3(0.f, 5.f, 5.f);

	bool matWorldDirty = false;

	DirectX::XMMATRIX matWorld{};
	DirectX::XMMATRIX matRot{};

public:
	/// @brief 追従の補間強度
	float easeRaito = 0.1f;

	/// @brief 親の回転を反映させるかどうか
	bool useParentRotaFlag = false;

public:
	/// @brief コンストラクタ
	/// @param parent 追従対象（親オブジェクト）
	CameraObj(AbstractGameObj* parent);

	inline void setEye2TargetOffset(const DirectX::XMFLOAT3& offset) { eye2TargetOffset = offset; }
	inline const auto& getEye2TargetOffset() const { return eye2TargetOffset; }

	/// @param eye2TargetLen カメラ位置から注視点の距離
	inline void setEye2TargetLen(float eye2TargetLen) { this->eye2TargetLen = eye2TargetLen; }
	/// @return カメラ位置から注視点の距離
	inline float getEye2TargetLen() const { return eye2TargetLen; }

	/// @return ワールド行列
	inline const DirectX::XMMATRIX& getMatWorld() const { return matWorld; }
	inline const DirectX::XMMATRIX& getMatRotation() const { return matRot; }

	/// @param parent 親オブジェクトのポインタ
	inline void setParentObj(AbstractGameObj* parent) { parentObj = parent; matWorldDirty = true; }
	/// @return 親オブジェクトのポインタ
	inline AbstractGameObj* getParentObj() const { return parentObj; }

	/// @param rotaDeg 親を基準とした回転
	inline void setRelativeRotaDeg(const DirectX::XMFLOAT3& rotaDeg) { relativeRotaDeg = rotaDeg; matWorldDirty = true; }
	/// @return 親を基準とした回転
	inline const DirectX::XMFLOAT3& getRelativeRotaDeg() const { return relativeRotaDeg; }

	void rotaCameraPos(const DirectX::XMFLOAT2& inputDeg);

private:

	void updateMatWorld();

	void preUpdate() override;
};
