#pragma once

#include "AbstractGameObj.h"

class BaseMovableObject :
	public AbstractGameObj
{
public:
	/// @param moveFlag 実際に移動させるかどうか
	/// @param outBuf 出力用変数
	void move(DirectX::XMFLOAT3& outBuf, const DirectX::XMVECTOR& dirNormal, float speed, bool moveYFlag = false, bool moveFlag = true);

	/// @param moveFlag 実際に移動させるかどうか
	/// @return 移動量
	inline DirectX::XMFLOAT3 move(const DirectX::XMVECTOR& dirNormal, float speed, bool moveYFlag = false, bool moveFlag = true)
	{
		DirectX::XMFLOAT3 ret{};
		move(ret, dirNormal, speed, moveYFlag, moveFlag);
		return ret;
	}

	/// @brief 視線方向に前進
	/// @param moveVel 移動量
	/// @param moveYFlag Y方向に移動するか
	void moveForward(float moveVel, bool moveYFlag = false);

	/// @brief 右に移動。前進処理のベクトルを右に90度傾けた移動。
	/// @param moveVel 移動量
	/// @param moveYFlag Y方向に移動するか
	void moveRight(float moveVel, bool moveYFlag = false);

	void moveParentRight(float moveVel, bool moveYFlag = false);

	/// @brief 上に移動。前進処理のベクトルを上に90度傾けた移動。
	/// @param moveVel 移動量
	void moveUp(float moveVel);

	void moveParentUp(float moveVel);
};
