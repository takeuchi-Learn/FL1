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
#include <System/PostEffect.h>
#include <Collision/Collision.h>
#include <Sound/Sound.h>

#include <Input/PadImu.h>

#include "TitleScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"

using namespace DirectX;

unsigned short PlayScene::stageNum = 0;

namespace
{
	constexpr auto bgmPath = "Resources/BGM/A-Sense-of-Loss.wav";
	constexpr auto particleGraphPath = L"Resources/judgeRange.png";
	constexpr auto billboardGraphPath = L"Resources/judgeRange.png";

	constexpr Timer::timeType transitionTime = Timer::oneSec;

	constexpr XMFLOAT2 lerp(const XMFLOAT2& s, const XMFLOAT2& e, float t)
	{
		return XMFLOAT2(std::lerp(s.x, e.x, t), std::lerp(s.y, e.y, t));
	}
}

void PlayScene::checkCollision()
{
	//for (auto& y : gameMap->getAABBs())
	//{
	//	for (auto& x : y)
	//	{
	//		// min、max両方0,0は通路なので確認せずにcontinue
	//		if (!checkMinMax(x)) { continue; }

	//		if (Collision::CheckHit(player->getShape(), x))
	//		{
	//			player->hit(x, typeid(*gameMap).name());
	//		}
	//	}
	//}

	for (auto& aabb : gameMap->getAABBs()) 
	{
		if (Collision::CheckHit(player->getShape(), aabb))
		{
			player->hit(aabb, typeid(*gameMap).name());
		}
	}

	// ゴールとプレイヤーの判定(仮)
	// 後々他のオブジェクトとまとめます
	const bool isHitGoal = Collision::CheckHit(player->getShape(), gameMap->getGoalAABB());

	if (isHitGoal)
	{
		player->hit(gameMap->getGoalAABB(), typeid(Goal).name());
	}
}

bool PlayScene::checkMinMax(const CollisionShape::AABB& aabb)
{
	return !(XMVectorGetX(aabb.minPos) == 0.f &&
			 XMVectorGetY(aabb.minPos) == 0.f &&
			 XMVectorGetX(aabb.maxPos) == 0.f &&
			 XMVectorGetY(aabb.maxPos) == 0.f);
}

PlayScene::PlayScene() :
	camera(std::make_unique<GameCamera>()),
	timer(std::make_unique<Stopwatch>())
{
	updateProc = std::bind(&PlayScene::update_start, this);

	bgm = Sound::ins()->loadWave(bgmPath);

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

	const auto mapYamlPath = "Resources/Map/map_" + std::to_string(stageNum) + ".yml";
	gameMap = std::make_unique<GameMap>(camera.get());

	XMFLOAT2 startPos{ 3.f, -1.f };
	const bool ret = gameMap->loadDataFile(mapYamlPath, &startPos);
	assert(false == ret);
	player->setMapPos(startPos);
	backGround = std::make_unique<BackGround>(camera.get(), static_cast<float>(gameMap->getMapY()));

	// ゲームオーバー扱いになる座標をセット(セットした値をプレイヤーの座標が下回ったら落下死)
	player->setGameOverPos(gameMap->getGameoverPos());
	player->setScrollendPosRight(static_cast<float>(gameMap->getMapX()) * 100.f - 1.f);

	// 開始時は物理挙動をしない
	player->isDynamic = false;
}

PlayScene::~PlayScene()
{
	Sound::stopWave(bgm);
}

void PlayScene::start()
{
	timer->reset();
}

void PlayScene::update()
{
	updateProc();

	camera->update();
	player->update();
	backGround->update();
	gameMap->update();

	// 衝突確認
	checkCollision();
}

void PlayScene::update_start()
{
	if (timer->getNowTime() > transitionTime)
	{
		player->isDynamic = true;
		PostEffect::ins()->setMosaicNum(XMFLOAT2((float)WinAPI::window_width, (float)WinAPI::window_height));
		PostEffect::ins()->setAlpha(1.f);
		Sound::playWave(bgm, XAUDIO2_LOOP_INFINITE, 0.2f);
		updateProc = std::bind(&PlayScene::update_main, this);
		timer->reset();
	} else
	{
		float raito = static_cast<float>(timer->getNowTime()) / static_cast<float>(transitionTime);
		//PostEffect::ins()->setAlpha(raito);
		//raito *= raito * raito * raito * raito;
		PostEffect::ins()->setMosaicNum(XMFLOAT2(raito * float(WinAPI::window_width),
												 raito * float(WinAPI::window_height)));
	}
}

void PlayScene::update_main()
{
	// スペースでシーン切り替え
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		if (!Input::ins()->hitKey(DIK_LCONTROL))
		{
			resetStageNum();
		}
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}

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
		++stageNum;
		SceneManager::ins()->changeScene<ClearScene>();
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