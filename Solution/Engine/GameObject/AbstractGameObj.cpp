#include "AbstractGameObj.h"

using namespace DirectX;

AbstractGameObj::AbstractGameObj(Camera* camera, ObjModel* model) :
	obj(std::make_unique<Object3d>(camera, model)),
	ppStateNum(Object3d::ppStateNum)
{}

AbstractGameObj::~AbstractGameObj()
{
	obj.reset(nullptr);
}

void AbstractGameObj::update()
{
	obj->update();
	for (auto& i : otherObj)
	{
		i.second->update();
	}

	for (auto& i : postUpdateProc)
	{
		i.second();
	}
}

void AbstractGameObj::draw(Light* light)
{
	obj->draw(light, ppStateNum);
	for (auto& i : otherObj)
	{
		i.second->draw(light);
	}

	for (auto& i : postDrawProc)
	{
		i.second(light);
	}
}

DirectX::XMFLOAT3 AbstractGameObj::calcVel(const DirectX::XMFLOAT3& targetPos, const DirectX::XMFLOAT3& nowPos, float velScale)
{
	XMFLOAT3 velF3 =
		XMFLOAT3(targetPos.x - nowPos.x,
				 targetPos.y - nowPos.y,
				 targetPos.z - nowPos.z);

	const XMVECTOR velVec =
		XMVectorScale(
			XMVector3Normalize(
				XMLoadFloat3(&velF3)
			),
			velScale
		);

	XMStoreFloat3(&velF3, velVec);
	return velF3;
}