#include "BaseMovableObject.h"

using namespace DirectX;

void BaseMovableObject::move(DirectX::XMFLOAT3& outBuf, const DirectX::XMVECTOR& dirNormal, float speed, bool moveYFlag, bool moveFlag)
{
	XMVECTOR velVec = XMVector3Transform(dirNormal, obj->getMatRotation());

	// Y方向に移動しないならY成分を消す
	if (!moveYFlag)
	{
		velVec = XMVector3Normalize(XMVectorSetY(velVec, 0.f));
	}

	// absがあるのは、大きさのみ指定したいから。
	// absがないと、moveVelがマイナスの場合に
	// マイナス * マイナスでプラスになってしまう
	velVec *= std::abs(speed);

	XMStoreFloat3(&outBuf, velVec);

	if (moveFlag)
	{
		obj->position.x += outBuf.x;
		obj->position.y += outBuf.y;
		obj->position.z += outBuf.z;
	}
}

void BaseMovableObject::moveForward(float moveVel, bool moveYFlag)
{
	move(XMVectorSet(0, 0, 1, 0), moveVel, moveYFlag);
}

void BaseMovableObject::moveRight(float moveVel, bool moveYFlag)
{
	move(XMVectorSet(1, 0, 0, 0), moveVel, moveYFlag);
}

void BaseMovableObject::moveParentRight(float moveVel, bool moveYFlag)
{
	if (!obj->parent)
	{
		moveRight(moveVel, moveYFlag);
		return;
	}

	// X方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(moveVel, 0, 0, 1), obj->parent->getMatRotation());

	// Y方向に移動しないならY成分を消す
	if (!moveYFlag)
	{
		// absがあるのは、大きさのみ指定したいから。
		// absがないと、moveVelがマイナスの場合に
		// マイナス * マイナスでプラスになってしまう
		velVec = XMVectorScale(XMVector3Normalize(XMVectorSetY(velVec, 0.f)),
							   std::abs(moveVel));
	}

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void BaseMovableObject::moveUp(float moveVel)
{
	// Y方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, moveVel, 0, 1), obj->getMatRotation());

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void BaseMovableObject::moveParentUp(float moveVel)
{
	if (!obj->parent)
	{
		moveUp(moveVel);
		return;
	}

	// Y方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, moveVel, 0, 1), obj->parent->getMatRotation());

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}