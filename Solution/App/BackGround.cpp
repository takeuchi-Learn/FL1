#include "BackGround.h"

#include<string>
#include<DirectXMath.h>

#include<GameCamera.h>
#include<GameObject/AbstractGameObj.h>
#include<3D/Obj/ObjModel.h>
#include<3D/Light/Light.h>

using namespace DirectX;

BackGround::BackGround(GameCamera* camera, const unsigned int stageNum)
	:camera(camera)
	, model(std::make_unique<ObjModel>
		(
			"Resources/backGround/back_" + std::to_string(stageNum) + "/",
			"back_" + std::to_string(stageNum)
		))
	,light(std::make_unique<Light>())
{
	obj = std::make_unique<AbstractGameObj>(camera, model.get());

	// 比率計算のためにテクスチャサイズ取得
	// 取得できないため一旦直接入力
	constexpr float scale = 4.f;
	obj->setScaleXY(XMFLOAT2(3840.f * scale,2000.f * scale));

	obj->setPosition(XMFLOAT3(1000.f, 1450.f, 10.f));
}

void BackGround::update()
{
	obj->update();

}

void BackGround::draw()
{
	obj->draw(light.get());
}
