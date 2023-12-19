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
#include <Sound/Sound.h>

#include <Player/Player.h>
#include <Object/Goal.h>
#include <BackGround.h>
#include<TutorialTexture.h>
#include <GameMap.h>
#include <System/PostEffect.h>
#include <Collision/Collision.h>
#include<ConeRecorder.h>

#include <Input/PadImu.h>

#include "TitleScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"

using namespace DirectX;

uint16_t PlayScene::stageNum = 0;

namespace
{
	constexpr auto bgmPath = "Resources/BGM/A-Sense-of-Loss.wav";
	constexpr auto particleGraphPath = L"Resources/judgeRange.png";
	constexpr auto billboardGraphPath = L"Resources/judgeRange.png";

	constexpr Timer::timeType transitionTime = Timer::oneSec;
	constexpr Timer::timeType approachTime = Timer::oneSec * 2;

	constexpr float cameraAngleDef = 30.f;

	constexpr float winWidth = static_cast<float>(WinAPI::window_width);
	constexpr float winHeight = static_cast<float>(WinAPI::window_height);
	constexpr float csHeight = (winHeight - (winWidth / 2.35f)) / 2.f;

	constexpr XMFLOAT2 lerp(const XMFLOAT2& s, const XMFLOAT2& e, float t)
	{
		return XMFLOAT2(std::lerp(s.x, e.x, t), std::lerp(s.y, e.y, t));
	}
}

void PlayScene::checkCollision()
{
	// 地形判定
	for (auto& aabb : gameMap->getMapAABBs())
	{
		if (Collision::CheckHit(player->getShape(), aabb))
		{
			player->hit(aabb, typeid(*gameMap).name());
		}
	}

	// オブジェクト判定
	const std::vector<std::unique_ptr<StageObject>>& objs = gameMap->getStageObjects();
	for (auto& obj : objs)
	{
		const CollisionShape::Sphere& sphere = player->getShape();
		const CollisionShape::AABB& aabb = obj->getRefAABB();
		if (Collision::CheckHit(sphere, aabb))
		{
			player->hit(aabb, typeid(*obj).name());
			obj->hit(sphere);
		}
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
	const auto blackTexNum = spriteBase->loadTexture(L"Resources/black.bmp");
	for (auto& i : cinemaScope)
	{
		i = std::make_unique<Sprite>(blackTexNum, spriteBase.get(), XMFLOAT2(0.f, 0.f));
		i->setSize(XMFLOAT2(winWidth, csHeight));
	}
	cinemaScope.back()->setAnchorPoint(XMFLOAT2(0.f, 1.f));
	constexpr float bottomY = static_cast<float>(WinAPI::window_height - 1);
	cinemaScope.back()->position.y = bottomY;

	camera->setEye(XMFLOAT3(0, 0, -5));
	camera->setTarget(XMFLOAT3(0, 0, 0));
	camera->setPerspectiveProjFlag(false);
	camera->setAngleDeg(cameraAngleDef);

	player = std::make_unique<Player>(camera.get());
	player->allowInput = false;
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
	player->setGameOverPos(gameMap->calcGameoverPos());
	player->setScrollendPosRight(static_cast<float>(gameMap->getMapX()) * 100.f - 1.f);

	// 開始時は物理挙動をしない
	player->isDynamic = false;

	// チュートリアル関係
	// もしチュートリアルステージ(_0、_1)だったら画像追加
	// ここで_0などの番号渡して画像を指定してもいいかもしれない(チュートリアルの画像名をtutorial_0みたいにして指定する)
	// 何番ステージまでがチュートリアルかを指定する
	// 一応_1までチュートリアルと仮定して1に設定
	constexpr unsigned short tutorialStageMax = 1;

	// チュートリアルステージだったら画像追加
	if (stageNum <= tutorialStageMax)
	{
		tutorialTexture = std::make_unique<TutorialTexture>(camera.get(), stageNum);
	}
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

	if (tutorialTexture)tutorialTexture->update();

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
		updateProc = std::bind(&PlayScene::update_approach, this);
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

void PlayScene::update_approach()
{
	const auto now = timer->getNowTime();
	if (now > approachTime)
	{
		for (auto& i : cinemaScope)
		{
			i->isInvisible = true;
		}
		camera->setAngleDeg(0.f);
		player->allowInput = true;
		updateProc = std::bind(&PlayScene::update_main, this);
		timer->reset();
		return;
	}
	const auto raito = static_cast<float>(now) / static_cast<float>(approachTime);
	camera->setAngleDeg(std::lerp(cameraAngleDef, 0.f, raito));
	const float height = std::lerp(csHeight, 0.f, raito);
	for (auto& i : cinemaScope)
	{
		i->setSize(XMFLOAT2(winWidth, height));
	}
}

void PlayScene::update_main()
{
#ifdef _DEBUG

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

#endif // _DEBUG

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

		// コーンのカウント記録
		ConeRecorder::getInstance()->registration(stageNum, player->getConeCount());

		// クリア演出後シーン切り替え
		SceneManager::ins()->changeScene<ClearScene>();
	}
}

void PlayScene::drawObj3d()
{
	backGround->draw();
	gameMap->draw();

	if (tutorialTexture) { tutorialTexture->draw(); }

	player->draw();
}

void PlayScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	for (auto& i : cinemaScope)
	{
		i->drawWithUpdate(DX12Base::ins(), spriteBase.get());
	}
}