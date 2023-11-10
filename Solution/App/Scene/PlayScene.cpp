﻿#include "PlayScene.h"
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

#include "TitleScene.h"

using namespace DirectX;

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

	playerModel = std::make_unique<ObjModel>("Resources/player", "player");
	player = std::make_unique<Player>(camera.get(), playerModel.get());
	player->setLight(light.get());

	Sound::ins()->playWave(bgm,
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);

	sensor = Sensor::create();
}

PlayScene::~PlayScene()
{
	Sound::ins()->stopWave(bgm);
	sensor->erase();
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
	camera->gameCameraUpdate(sensor);
	light->update();
	sensor->update();
}

void PlayScene::drawObj3d()
{
	player->draw();
	particle->drawWithUpdate();
}

void PlayScene::drawFrontSprite()
{
	const auto mousePos = Input::ins()->calcMousePosFromSystem();
	sprite->position.x = mousePos.x;
	sprite->position.y = mousePos.y;

	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	sprite->drawWithUpdate(DX12Base::ins(), spriteBase.get());

	using namespace ImGui;

	SetNextWindowPos(ImVec2(0.f, 0.f));

	Begin("PlaySceneGUI", nullptr, DX12Base::imGuiWinFlagsDef);
	Text("タイマー%s: %f",
		 stopwatch->isPlay() ? "|>" : "□",
		 float(stopwatchPlayTime) / Timer::oneSecF);
	Text("スペース: シーン切り替え");
	Text("x:%.1f, y:%.1f, z:%.1f",
		camera->getEye().x,
		camera->getEye().y,
		camera->getEye().z);
	Text("Accel : x:%.1f, y:%.1f, z:%.1f",
		 sensor->GetAccelX(),
		 sensor->GetAccelY(),
		 sensor->GetAccelZ());
	Text("Gyro : x:%.1f, y:%.1f, z:%.1f",
		 sensor->GetGyroX(),
		 sensor->GetGyroY(),
		 sensor->GetGyroZ());
	Text("Angle : %.1f",
		 camera->getAngle());
	End();
}