#include "BackGround.h"

#include<string>
#include<DirectXMath.h>

#include<GameCamera.h>
#include<3D/Obj/ObjModel.h>

using namespace DirectX;

BackGround::BackGround(GameCamera* camera,const unsigned int stageNum)
	:camera(camera)
	, obj(std::make_unique<Billboard>((L"Resources/backGround/back_" + std::to_wstring(stageNum) + L".png").c_str(), camera))
{
	model = std::unique_ptr<ObjModel>();
	
}

void BackGround::update()
{
	obj->update(XMConvertToRadians(-camera->getAngleDeg()));
}

void BackGround::draw()
{
	obj->draw();
}
