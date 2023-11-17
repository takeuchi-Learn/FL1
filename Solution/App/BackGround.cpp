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

	// 平行投影の場合、相当カメラ離したほうがいい(デフォルト猿モデルだとeyeのZ値-600くらいがベスト)


	// Xを指定したら自動的にYを計算するようにする
	constexpr float scaleX = 4000.f;
	
	obj->setScaleXY(XMFLOAT2(scaleX,1000.f));
	obj->setPosition(XMFLOAT3(0, 0, -1200));
}

void BackGround::update()
{
	obj->update();
}

void BackGround::draw()
{
	obj->draw(light.get());
}
