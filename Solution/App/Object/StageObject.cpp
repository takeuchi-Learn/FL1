#include "StageObject.h"
#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>
#include<Player/Player.h>
using namespace DirectX;

float StageObject::calcCameraAngleRad()
Player* StageObject::player;

float StageObject::getCameraAngleDeg()
{
	return XMConvertToRadians(-camera->getAngleDeg());
}

StageObject::StageObject(GameCamera* camera, const DirectX::XMFLOAT2& pos, const DirectX::XMFLOAT2& scale, const std::wstring& texPath)
	:gameObj(std::make_unique<Billboard>(texPath.c_str(), camera))
	, camera(camera)
{
	gameObj->add(XMFLOAT3(pos.x, pos.y, 0), scale);

	// AABBのサイズ(一辺の長さ)
	// 画像サイズ関係なくグリッド座標基準で求めている
	constexpr auto harfScale = XMFLOAT2(100.f / 2.f, 100.f / 2.f);
	aabb.minPos = XMVectorSet(pos.x - harfScale.x, pos.y - harfScale.y, 0.f, 0.f);
	aabb.maxPos = XMVectorSet(pos.x + harfScale.x, pos.y + harfScale.y, 0.f, 0.f);
}

void StageObject::update()
{
	gameObj->update(calcCameraAngleRad(), center);
}

void StageObject::draw()
{
	gameObj->draw();
}