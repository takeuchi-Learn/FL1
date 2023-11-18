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
#include <Object/Goal.h>
#include <BackGround.h>
#include <GameMap.h>

#include <Collision/Collision.h>
#include <PadImu.h>
#include <JoyShockLibrary.h>

#include "TitleScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"

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

void PlayScene::checkCollision()
{
	const auto& mapAABBs = gameMap->getAABBs();
	for (auto y = 0; y < mapAABBs.size(); y++)
	{
		for (auto x = 0; x < mapAABBs[y].size(); x++)
		{
			if (Collision::CheckSphere2AABB(player->getShape(), mapAABBs[y][x]))
			{
				player->hit(mapAABBs[y][x], typeid(*gameMap).name());
			}
		}
	}

	// テストです
	//bool res = Collision::CheckSphere2AABB(player->getShape(), goal->getShape());
	//if (res)
	//{
	//	//player->hit();
	//}

}

PlayScene::PlayScene() :
	camera(std::make_unique<GameCamera>())
{
	spriteBase = std::make_unique<SpriteBase>();
	sprite = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/judgeRange.png"),
									  spriteBase.get(),
									  XMFLOAT2(0.5f, 0.5f));
	sprite->color.w = 0.5f;

	camera->setEye(XMFLOAT3(0, 0, -5));
	camera->setTarget(XMFLOAT3(0, 0, 0));
	camera->setPerspectiveProjFlag(false);

	player = std::make_unique<Player>(camera.get());
	// 追従させるためにポインタを渡す
	camera->setParentObj(player->getObj().get());

	// 仮にプレイヤーモデルを割り当て
	//constexpr DirectX::XMFLOAT2 goalPos(80, 0);
	//goal = std::make_unique<Goal>(nullptr, camera.get(),  goalPos);

	backGround = std::make_unique<BackGround>(camera.get());

	gameMap = std::make_unique<GameMap>(camera.get());
	const bool ret = gameMap->loadDataFile(mapYamlPath);
	assert(false == ret);

	// ゲームオーバー扱いになる座標をセット(セットした値をプレイヤーの座標が下回ったら落下死)
	player->setGameOverPos(gameMap->getGameoverPos());

}

PlayScene::~PlayScene()
{}

void PlayScene::update()
{
	// スペースでシーン切り替え
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}

	// todo ジャンプのための加速度入力仮
	if (PadImu::ins()->getDevCount() > 0)
	{
		const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);
		player->setSensorValue(state.accelY);
	}

	backGround->update();
	gameMap->update();
	player->update();

	// ライトとカメラの更新
	camera->update();
	camera->gameCameraUpdate();

	// 衝突確認
	checkCollision();

	// ゲームオーバー確認
	if (player->getIsDead())
	{
		camera->changeStateGameover();

		gameOverTimer++;

		// 一定時間後にゲームオーバーシーン切り替え
		if (gameOverTimer >= GAME_OVER_TIME_MAX)
		{
			SceneManager::ins()->changeScene<GameOverScene>();
		}
	}

	// クリア確認
	if (player->getIsClear())
	{
		camera->changeStateClear();
		// クリア演出後シーン切り替え
		if (1)
		{
			SceneManager::ins()->changeScene<ClearScene>();
		}
	}
}

void PlayScene::drawObj3d()
{
	backGround->draw();
	gameMap->draw();
	player->draw();
	//goal->draw();
}

void PlayScene::drawFrontSprite()
{
	// 対応パッドが無ければ何も表示しない
	if (PadImu::ins()->getDevCount() <= 0) { return; }

	const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);

	using namespace ImGui;
	Begin("PlayScene::drawFrontSprite()", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
	Text("accelY: %.1f", state.accelY);
	End();
}