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

#include "TitleScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"

using namespace DirectX;

unsigned short PlayScene::stageNum = 0;

namespace
{
	constexpr auto bgmPath = "Resources/BGM.wav";
	constexpr auto particleGraphPath = L"Resources/judgeRange.png";
	constexpr auto billboardGraphPath = L"Resources/judgeRange.png";
	//constexpr auto mapYamlPath = "Resources/Map/map.yml";

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
			// min、max両方0,0は通路なので確認せずにcontinue
			if (!checkMinMax(mapAABBs[y][x]))continue;

			if (Collision::CheckSphere2AABB(player->getShape(), mapAABBs[y][x]))
			{
				player->hit(mapAABBs[y][x], typeid(*gameMap).name());
			}
		}
	}

	// ゴールとプレイヤーの判定(仮)
	// 後々他のオブジェクトとまとめます
	if (Collision::CheckSphere2AABB(player->getShape(), gameMap->getGoalAABB()))
	{
		player->hit(gameMap->getGoalAABB(), typeid(Goal).name());
	}

	// テストです
	//bool res = Collision::CheckSphere2AABB(player->getShape(), goal->getShape());
	//if (res)
	//{
	//	//player->hit();
	//}

}

bool PlayScene::checkMinMax(const CollisionShape::AABB& aabb)
{
	XMFLOAT2 minPos(aabb.minPos.m128_f32[0], aabb.minPos.m128_f32[1]);
	XMFLOAT2 maxPos(aabb.maxPos.m128_f32[0], aabb.maxPos.m128_f32[1]);
	if (minPos.x == 0.f && minPos.y == 0.f && maxPos.x == 0.f && maxPos.y == 0.f)return false;
	return true;
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

	const auto mapYamlPath = "Resources/Map/map_" + std::to_string(stageNum) +".yml";
	gameMap = std::make_unique<GameMap>(camera.get());
	const bool ret = gameMap->loadDataFile(mapYamlPath);
	assert(false == ret);

	// ゲームオーバー扱いになる座標をセット(セットした値をプレイヤーの座標が下回ったら落下死)
	player->setGameOverPos(gameMap->getGameoverPos());
	player->setGoalPosX(gameMap->getGoalPosX());

}

PlayScene::~PlayScene()
{
}

void PlayScene::update()
{
	// スペースでシーン切り替え
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		resetStageNum();
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}


	// ライトとカメラの更新
	camera->gameCameraUpdate();
	camera->update();

	player->update();
	backGround->update();
	gameMap->update();

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
	if(player->getIsClear())
	{
		camera->changeStateClear();
		// クリア演出後シーン切り替え
		if(1)
		{
			stageNum++;
			SceneManager::ins()->changeScene<ClearScene>();
		}
	}
}

void PlayScene::drawObj3d()
{
	backGround->draw();
	gameMap->draw();
	player->draw();
}

void PlayScene::drawFrontSprite()
{}