#include "BackGround.h"

#include<string>
#include<DirectXMath.h>

#include<GameCamera.h>
#include<3D/Billboard/Billboard.h>

using namespace DirectX;

BackGround::BackGround(GameCamera* camera,const unsigned int stageNum)
	:camera(camera)
	, backGround(std::make_unique<Billboard>((L"Resources/backGround/back_" + std::to_wstring(stageNum) + L".png").c_str(), camera))
{
	constexpr float scale = 1000.f;
	backGround->add(XMFLOAT3(), scale, 0.f);
	
}

void BackGround::update()
{
	backGround->update(XMConvertToRadians(-camera->getAngleDeg()));
}

void BackGround::draw()
{
	backGround->draw();
}
