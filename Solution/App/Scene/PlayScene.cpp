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
#include <Player/Player.h>
#include <Object/Goal.h>

#include <Collision/Collision.h>

#include"../GameCamera.h"
#include "../GameMap.h"
#include "TitleScene.h"
#include <3D/Light/Light.h>

using namespace DirectX;

void PlayScene::checkCollision()
{
	// テストです
	bool res = Collision::CheckSphere2AABB(player->getShape(), goal->getShape());
	if(res)
	{
		player->hit();
	}

}

PlayScene::PlayScene() :
	light(std::make_unique<Light>()),
	camera(std::make_unique<GameCamera>()),
	stopwatch(std::make_unique<Stopwatch>()),
	stopwatchPlayTime(Timer::timeType(0u))
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

	player = std::make_unique<Player>(camera.get());

	camera->setParentObj(player->getObj().get());

	playerModel = std::make_unique<ObjModel>("Resources/player", "player");
	player = std::make_unique<Player>(camera.get(), playerModel.get());
	player->setLight(light.get());

	// 仮にプレイヤーモデルを割り当て
	constexpr DirectX::XMFLOAT2 goalPos(80, 0);
	goal = std::make_unique<Goal>( nullptr, camera.get(), light.get(), goalPos);

	Sound::ins()->playWave(bgm,
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);
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
	player->update();

	// ライトとカメラの更新
	camera->update();
	camera->gameCameraUpdate();
	light->update();

	// 衝突判定テスト
	checkCollision();
}

void PlayScene::drawObj3d()
{
	gameMap->draw();
	player->draw();
	goal->draw();
	particle->drawWithUpdate();
}

void PlayScene::drawFrontSprite() {}