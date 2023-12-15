#include "StageObject.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>
using namespace DirectX;

float StageObject::calcCameraAngleRad()
{
	return XMConvertToRadians(-camera->getAngleDeg());
}

StageObject::StageObject(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale, const std::wstring& texPath)
	:gameObj(std::make_unique<Billboard>(texPath.c_str(), camera))
	,camera(camera)
{
	gameObj->add(XMFLOAT3(pos.x, pos.y, 0), scale);

	// AABBのサイズ(一辺の長さ)
	const float harfScale = scale / 2.f;
	XMFLOAT2 minPos(pos.x - harfScale, pos.y - harfScale);
	XMFLOAT2 maxPos(pos.x + harfScale, pos.y + harfScale);
	aabb.minPos = XMLoadFloat2(&minPos);
	aabb.maxPos = XMLoadFloat2(&maxPos);
}

void StageObject::update()
{
	gameObj->update(calcCameraAngleRad());
}

void StageObject::draw()
{
	gameObj->draw();
}


