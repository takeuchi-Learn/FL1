﻿#include "BackGround.h"

#include <string>
#include <DirectXMath.h>

#include <GameCamera.h>
#include <GameObject/AbstractGameObj.h>
#include <3D/Obj/ObjModel.h>
#include <3D/Light/Light.h>

using namespace DirectX;

BackGround::BackGround(GameCamera* camera, const float mapY, const unsigned int stageNum)
	: camera(camera)
	, light(std::make_unique<Light>())
	, model(std::make_unique<ObjModel>("Resources/backGround/back_" + std::to_string(stageNum) + "/",
									   "back_" + std::to_string(stageNum)))
{
	obj = std::make_unique<AbstractGameObj>(camera, model.get());

	// 比率計算のためにテクスチャサイズ取得
	// 取得できないため一旦直接入力
	constexpr float scale = 2.5f;
	obj->setScaleXY(XMFLOAT2(3840.f * scale, 2000.f * scale));

	const float y = 1200.f + 64.f * -mapY;
	obj->setPosition(XMFLOAT3(6000.f, 1200.f + 64.f * -mapY, 10.f));
}

void BackGround::update()
{
	obj->update();
}

void BackGround::draw()
{
	obj->draw(light.get());
}