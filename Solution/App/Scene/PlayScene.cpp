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

	constexpr Timer::timeType transitionTime = Timer::oneSec;

	constexpr XMFLOAT2 lerp(const XMFLOAT2& s, const XMFLOAT2& e, float t)
	{
		return XMFLOAT2(std::lerp(s.x, e.x, t), std::lerp(s.y, e.y, t));
	}
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
}

PlayScene::PlayScene() :
	camera(std::make_unique<GameCamera>()),
	timer(std::make_unique<Stopwatch>())
{
	updateProc = std::bind(&PlayScene::update_start, this);

	bgm = Sound::ins()->loadWave("Resources/A-Sense-of-Loss.wav");

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

	backGround = std::make_unique<BackGround>(camera.get());

	gameMap = std::make_unique<GameMap>(camera.get());
	const bool ret = gameMap->loadDataFile(mapYamlPath);
	assert(false == ret);

	// ゲームオーバー扱いになる座標をセット(セットした値をプレイヤーの座標が下回ったら落下死)
	player->setGameOverPos(gameMap->getGameoverPos());

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

	// 衝突確認
	checkCollision();

	backGround->update();
	gameMap->update();
	player->update();

	// ライトとカメラの更新
	camera->update();
	camera->gameCameraUpdate();
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
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}

	// todo ジャンプのための加速度入力仮
	if (PadImu::ins()->getDevCount() > 0)
	{
		const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);
		player->setSensorValue(state.accelY);
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
#ifdef _DEBUG

	// 対応パッドが無ければ何も表示しない
	if (PadImu::ins()->getDevCount() <= 0) { return; }

	const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);

	using namespace ImGui;
	Begin("PlayScene::drawFrontSprite()", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
	Text("accelY: %.1f", state.accelY);
	End();

#endif // _DEBUG
}