#include "ColorCone.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>
void ColorCone::blownAway()
{
	if (!isBlownAway)return;



}


ColorCone::ColorCone(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale)
	:StageObject(camera, pos, scale, L"Resources/Map/Tex/goal.png")
{

}

void ColorCone::update()
{
	blownAway();
}

void ColorCone::hit(const CollisionShape::Sphere & playerSphere)
{
	// 吹っ飛ぶ処理有効
	isBlownAway = true;
}
