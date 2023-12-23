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
#include <ConeRecorder.h>

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
	constexpr Timer::timeType appearanceTime = Timer::oneSec * 2;

	constexpr float cameraAngleDef = 30.f;

	constexpr auto winSize = XMFLOAT2(static_cast<float>(WinAPI::window_width), static_cast<float>(WinAPI::window_height));
	constexpr float csHeight = (winSize.y - (winSize.x / 2.35f)) / 2.f;

	constexpr XMFLOAT2 lerp(const XMFLOAT2& s, const XMFLOAT2& e, float t)
	{
		return XMFLOAT2(std::lerp(s.x, e.x, t), std::lerp(s.y, e.y, t));
	}
}

PlayScene::PlayScene() :
	camera(std::make_unique<GameCamera>()),
	timer(std::make_unique<Stopwatch>()),
	gauge(std::make_unique<CircularGauge>())
{
	updateProc = std::bind(&PlayScene::update_start, this);
	updateCinemaScopeProc = [] {};

	bgm = Sound::ins()->loadWave(bgmPath);

	spriteBase = std::make_unique<SpriteBase>();
	const auto blackTexNum = spriteBase->loadTexture(L"Resources/black.bmp");
	for (auto& i : cinemaScope)
	{
		i = std::make_unique<Sprite>(blackTexNum, spriteBase.get(), XMFLOAT2(0.f, 0.f));
		i->setSize(XMFLOAT2(winSize.x, csHeight));
	}
	cinemaScope.back()->setAnchorPoint(XMFLOAT2(0.f, 1.f));
	constexpr float bottomY = static_cast<float>(WinAPI::window_height - 1);
	cinemaScope.back()->position.y = bottomY;

	camera->setEye(XMFLOAT3(0, 0, -5));
	camera->setTarget(XMFLOAT3(0, 0, 0));
	camera->setPerspectiveProjFlag(false);
	camera->setAngleDeg(cameraAngleDef);
	camera->allowInput = false;

	player = std::make_unique<Player>(camera.get());
	player->allowInput = false;
	player->isDynamic = false;
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

	gauge->setCamera(camera.get());
	gaugeData = gauge->add({}, 100.f, XMFLOAT4(0.f, 1.f, 1.f, 1.f));

	// チュートリアル関係
	// もしチュートリアルステージ(_0、_1)だったら画像追加
	// ここで_0などの番号渡して画像を指定してもいいかもしれない(チュートリアルの画像名をtutorial_0みたいにして指定する)
	// 何番ステージまでがチュートリアルかを指定する
	// 一応_1までチュートリアルと仮定して1に設定
	constexpr uint16_t tutorialStageMax = 1;

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
	for (auto& obj : gameMap->getStageObjects())
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

	if (auto i = gaugeData.lock(); i)
	{
		i->position = player->getObj()->position;
		i->position.z -= 0.1f;
		// 大きさは自機
		i->scale = player->getObj()->scale;

		i->gaugeRaito = std::fmod(i->gaugeRaito + 0.02f, 1.f);
	}
	gauge->update();
}

void PlayScene::update_start()
{
	if (timer->getNowTime() > transitionTime)
	{
		// 自機は物理挙動する
		// 演出中に動くのが嫌なら`update_appearance`関数の`allowInput=true`があるところに移動する
		player->isDynamic = true;

		PostEffect::ins()->setMosaicNum(winSize);
		PostEffect::ins()->setAlpha(1.f);
		Sound::playWave(bgm, XAUDIO2_LOOP_INFINITE, 0.2f);
		updateProc = std::bind(&PlayScene::update_appearance, this);
		timer->reset();
	} else
	{
		float raito = static_cast<float>(timer->getNowTime()) / static_cast<float>(transitionTime);
		//PostEffect::ins()->setAlpha(raito);
		//raito *= raito * raito * raito * raito;
		PostEffect::ins()->setMosaicNum(XMFLOAT2(raito * winSize.x,
												 raito * winSize.y));
	}
}

void PlayScene::update_appearance()
{
	const auto now = timer->getNowTime();
	if (now > appearanceTime)
	{
		// シネスコをだんだん消す
		updateCinemaScopeProc = [&]
			{
				constexpr auto maxTime = Timer::oneSec / 2;
				constexpr float maxTimeF = static_cast<float>(maxTime);
				const auto now = timer->getNowTime();
				if (now >= maxTime)
				{
					// 終わったらシネスコは非表示に、シネスコ更新関数は空にする。
					for (auto& i : cinemaScope)
					{
						i->isInvisible = true;
					}
					updateCinemaScopeProc = [] {};
					return;
				}
				const auto raito = static_cast<float>(now) / maxTimeF;
				const float height = std::lerp(csHeight, 0.f, raito);
				for (auto& i : cinemaScope)
				{
					i->setSize(XMFLOAT2(winSize.x, height));
				}
			};

		camera->setAngleDeg(0.f);
		camera->allowInput = true;
		player->allowInput = true;
		updateProc = std::bind(&PlayScene::update_main, this);
		timer->reset();
		return;
	}
	const auto raito = static_cast<float>(now) / static_cast<float>(appearanceTime);
	camera->setAngleDeg(std::lerp(cameraAngleDef, 0.f, raito));
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

	// シネスコ更新関数
	updateCinemaScopeProc();

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

	gauge->draw();
}

void PlayScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	for (auto& i : cinemaScope)
	{
		i->drawWithUpdate(DX12Base::ins(), spriteBase.get());
	}
}