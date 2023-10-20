#include "PlayScene.h"
#include <System/DX12Base.h>
#include <System/SceneManager.h>
#include <Input/Input.h>
#include <Camera/Camera.h>
#include <3D/Light/Light.h>
#include <3D/Obj/Object3d.h>
#include <3D/Obj/ObjModel.h>
#include <2D/Sprite.h>
#include <Sound/Sound.h>
#include <Sound/SoundData.h>
#include <3D/ParticleMgr.h>
#include <3D/Billboard/Billboard.h>
#include <algorithm>

#include"../GameCamera.h"
#include "TitleScene.h"

using namespace DirectX;

namespace
{
	constexpr auto bgmPath = "Resources/BGM.wav";
	constexpr auto particleGraphPath = L"Resources/judgeRange.png";
	constexpr auto billboardGraphPath = L"Resources/judgeRange.png";

	constexpr XMFLOAT3 objectPosDef = XMFLOAT3(0, 0, 0);
	constexpr XMFLOAT3 cameraPosDef = XMFLOAT3(0, 0, -600);

	constexpr size_t mapX = 5;
	constexpr size_t mapY = 5;
	constexpr uint8_t map[mapY][mapX] = {
		{1,1,1,1,1},
		{1,0,1,1,1},
		{1,0,1,1,1},
		{1,0,0,0,1},
		{1,1,1,1,1},
	};
}

PlayScene::PlayScene() :
	light(std::make_unique<Light>()),
	camera(std::make_unique<GameCamera>())
{
	camera->setEye(cameraPosDef);
	camera->setTarget(objectPosDef);
	camera->setPerspectiveProjFlag(false);

	billboards = std::make_unique<Billboard>(billboardGraphPath, camera.get());
	for (size_t y = 0; y < mapY; y++)
	{
		for (size_t x = 0; x < mapX; x++)
		{
			if (0 == map[y][x]) { continue; }

			constexpr float scale = 100.f;

			const auto pos = XMFLOAT3(float(x) * scale - (scale * 2.f), -float(y) * scale + scale, 0);

			// todo アルファが0.5以下だと表示されない
			const auto color = XMFLOAT4(float(x) / mapX, float(y) / mapY, 1, 1);

			billboards->add(pos, scale, 0.f, color);
		}
	}
}

PlayScene::~PlayScene() {}

void PlayScene::update()
{
	// スペースでシーン切り替え
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}

	// 更新
	camera->gameCameraUpdate();
	camera->update();
	light->update();

	billboards->update(XMConvertToRadians(-camera->getAngle()));
}

void PlayScene::drawObj3d()
{
	billboards->draw();
}

void PlayScene::drawFrontSprite() {}