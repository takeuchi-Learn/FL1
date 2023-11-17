#include "Goal.h"


Goal::Goal(ObjModel* model, Camera* camera,  const DirectX::XMFLOAT2& pos)
	:gameObj(std::make_unique<AbstractGameObj>(camera,model))
{
	gameObj->setPosition(DirectX::XMFLOAT3(pos.x, pos.y, 0));

	// AABBのサイズ(一辺の長さ)
	constexpr float size = 5.0f;
	aabb.minPos.m128_f32[0] = pos.x - size / 2;
	aabb.minPos.m128_f32[1] = pos.y - size / 2;
	
	aabb.maxPos.m128_f32[0] = pos.x + size / 2;
	aabb.maxPos.m128_f32[1] = pos.y + size / 2;

}

void Goal::draw()
{
	gameObj->draw(light);
}
