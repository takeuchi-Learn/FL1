#include "Goal.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>
using namespace DirectX;

Goal::Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, const float scale)
	:StageObject(camera, pos, scale)
{
}