#include "Goal.h"


#include<GameCamera.h>
#include<3D/Billboard/Billboard.h>
using namespace DirectX;

Goal::Goal(GameCamera* camera,  const DirectX::XMFLOAT2& pos,const float scale)
	:gameObj(std::make_unique<Billboard>(L"Resources/Map/Tex/goal.png", camera))
	, camera(camera)
{
	gameObj->add
	(
		XMFLOAT3(pos.x, pos.y, 0),
		scale
	);

	// AABBのサイズ(一辺の長さ)
	const float harfScale = scale / 2;
	XMFLOAT2 minPos(pos.x - harfScale, pos.y - harfScale);
	XMFLOAT2 maxPos(pos.x + harfScale, pos.y + harfScale);
	aabb.minPos = XMLoadFloat2(&minPos);
	aabb.maxPos = XMLoadFloat2(&maxPos);

}

void Goal::update()
{
	gameObj->update(XMConvertToRadians(-camera->getAngleDeg()));
}

void Goal::draw()
{
	gameObj->draw();
}
