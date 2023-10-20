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
#include "../GameMap.h"
#include "TitleScene.h"

using namespace DirectX;

namespace
{
	constexpr auto bgmPath = "Resources/BGM.wav";
	constexpr auto particleGraphPath = L"Resources/judgeRange.png";
	constexpr auto billboardGraphPath = L"Resources/judgeRange.png";
	constexpr auto mapYamlPath = "Resources/Map/map.yml";

	constexpr XMFLOAT3 objectPosDef = XMFLOAT3(0, 0, 0);
	constexpr XMFLOAT3 cameraPosDef = XMFLOAT3(0, 0, -600);
}

PlayScene::PlayScene() :
	camera(std::make_unique<GameCamera>())
{
	camera->setEye(cameraPosDef);
	camera->setTarget(objectPosDef);
	camera->setPerspectiveProjFlag(false);

	gameMap = std::make_unique<GameMap>(camera.get());
	const bool ret = gameMap->loadDataFile(mapYamlPath);
	assert(false == ret);
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
	camera->update();
	gameMap->update();
}

void PlayScene::drawObj3d()
{
	gameMap->draw();
}

void PlayScene::drawFrontSprite() {}