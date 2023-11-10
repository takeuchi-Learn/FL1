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
#include <algorithm>
#include <Player/Player.h>
#include <Object/Goal.h>
#include<GameMap.h>

#include <Collision/Collision.h>

#include "TitleScene.h"

using namespace DirectX;

void PlayScene::checkCollision()
{
	// テストです
	bool res = Collision::CheckSphere2AABB(player->getShape(), goal->getShape());
	if (res)
	{
		//player->hit();
	}

}

PlayScene::PlayScene() :
	light(std::make_unique<Light>()),
	camera(std::make_unique<GameCamera>()),
	stopwatch(std::make_unique<Stopwatch>()),
	stopwatchPlayTime(Timer::timeType(0u))
{
	bgm = Sound::ins()->loadWave("Resources/BGM.wav");

	spriteBase = std::make_unique<SpriteBase>();
	sprite = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/judgeRange.png"),
									  spriteBase.get(),
									  XMFLOAT2(0.5f, 0.5f));
	sprite->color.w = 0.5f;

	camera->setEye(XMFLOAT3(0, 0, -5));
	camera->setTarget(XMFLOAT3(0, 0, 0));

	light->setDirLightActive(0u, true);
	light->setDirLightDir(0u, XMVectorSet(1, 0, 0, 0));
	light->setPointLightPos(0u, camera->getEye());

	particle = std::make_unique<ParticleMgr>(L"Resources/judgeRange.png", camera.get());

	player = std::make_unique<Player>(camera.get());

	// 仮にプレイヤーモデルを割り当て
	constexpr DirectX::XMFLOAT2 goalPos(80, 0);
	goal = std::make_unique<Goal>(nullptr, camera.get(), light.get(), goalPos);

	Sound::ins()->playWave(bgm,
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);
}

PlayScene::~PlayScene()
{
	Sound::ins()->stopWave(bgm);
}

void PlayScene::update()
{
	// スペースでシーン切り替え
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}

	if (Input::ins()->triggerKey(DIK_P))
	{
		constexpr float particleVel = 0.1f;
		particle->createParticle({ 0,0,0 },
								 10u,
								 1.f,
								 particleVel);
	}

	if (Input::ins()->triggerKey(DIK_B))
	{
		static bool isPlay = true;

		if (isPlay)
		{
			bgm.lock()->stopVoice();
		} else
		{
			bgm.lock()->startVoice();
		}
		isPlay = !isPlay;
	}

	// Tを押したらタイマーを停止/再開
	if (Input::ins()->triggerKey(DIK_T))
	{
		stopwatch->stopOrResume();
	}
	// 測定中なら現在時間を更新
	if (stopwatch->isPlay())
	{
		stopwatchPlayTime = stopwatch->getNowTime();
	}

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
	player->draw();
	goal->draw();
	//gameMap->draw();
	particle->drawWithUpdate();
}

void PlayScene::drawFrontSprite()
{
	const auto mousePos = Input::ins()->calcMousePosFromSystem();
	sprite->position.x = mousePos.x;
	sprite->position.y = mousePos.y;

	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	sprite->drawWithUpdate(DX12Base::ins(), spriteBase.get());
}