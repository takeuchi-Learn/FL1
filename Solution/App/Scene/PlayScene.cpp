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
}

PlayScene::PlayScene() :
	light(std::make_unique<Light>()),
	camera(std::make_unique<GameCamera>())
{
	camera->setEye(cameraPosDef);
	camera->setTarget(objectPosDef);

	billboards = std::make_unique<Billboard>(billboardGraphPath, camera.get());
	billboards->add(objectPosDef, 100.f, 0.f, XMFLOAT3(0, 1, 1));
	billboards->add(XMFLOAT3(0, 50, 0), 100.f, 0.f, XMFLOAT3(1, 1, 1));
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

	billboards->update();
}

void PlayScene::drawObj3d()
{
	billboards->draw();
}

void PlayScene::drawFrontSprite() {}