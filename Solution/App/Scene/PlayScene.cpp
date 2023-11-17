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
#include <BackGround.h>
#include <GameMap.h>

#include <Collision/Collision.h>

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

void PlayScene::checkCollision()
{
	const auto& mapAABBs = gameMap->getAABBs();
	// テストです
	for(auto y = 0; y < mapAABBs.size();y++)
	{
		// テストのため一部マップチップを無視
		if (y == 0|| y == 1)continue;

		for (auto x = 0; x < mapAABBs[y].size(); x++)
		{
			// テスト
			
			if (y == 2 && !(x == 4 ||x == 0))
			{
				continue;
			}

			if(Collision::CheckSphere2AABB(player->getShape(), mapAABBs[y][x]))
			{
				player->hit(mapAABBs[y][x]);
			}
		}
	}

	// テストです
	bool res = Collision::CheckSphere2AABB(player->getShape(), goal->getShape());
	if (res)
	{
		//player->hit();
	}

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
	constexpr DirectX::XMFLOAT2 goalPos(80, 0);
	goal = std::make_unique<Goal>(nullptr, camera.get(),  goalPos);

	backGround = std::make_unique<BackGround>(camera.get());

	gameMap = std::make_unique<GameMap>(camera.get());
	const bool ret = gameMap->loadDataFile(mapYamlPath);
	assert(false == ret);
}

PlayScene::~PlayScene()
{
}

void PlayScene::update()
{
	// スペースでシーン切り替え
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}

	backGround->update();
	gameMap->update();
	player->update();

	// ライトとカメラの更新
	camera->update();
	camera->gameCameraUpdate();

	// 衝突判定テスト
	checkCollision();
}

void PlayScene::drawObj3d()
{
	gameMap->draw();
	player->draw();
	goal->draw();
	backGround->draw();
}

void PlayScene::drawFrontSprite()
{}