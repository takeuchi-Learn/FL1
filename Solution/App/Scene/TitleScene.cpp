#include "TitleScene.h"
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <Util/Timer.h>
#include <Input/PadImu.h>
#include <2D/Sprite.h>
#include <2D/SpriteBase.h>
#include <Util/Util.h>
#include <Sound/Sound.h>
#include <algorithm>

#include "PlayScene.h"
#include "StageSelectScene.h"

using namespace DirectX;

namespace
{
	constexpr float transitionTimeSec = 1.5f;
	constexpr auto transitionTime = static_cast<Timer::timeType>(Timer::oneSecF * transitionTimeSec);
	constexpr float endPos = static_cast<float>(WinAPI::window_height) + 10.f;

	constexpr auto tutoGraph = L"Resources/title/tutorial_oji.png";
}

TitleScene::TitleScene()
{
	spBase = std::make_unique<SpriteBase>();
	backSprite = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/title/Title_Back.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	logoSprite = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/title/Title_logo.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	nowLoading = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/nowLoading.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	nowLoading->isInvisible = true;
	operation = std::make_unique<Sprite>(spBase->loadTexture(tutoGraph), spBase.get(), XMFLOAT2(0.f, 0.f));
	operation->isInvisible = true;
	operation->position.y = endPos;

	constexpr XMFLOAT2 winSize = XMFLOAT2(float(WinAPI::window_width), float(WinAPI::window_height));
	backSprite->setSize(winSize);
	logoSprite->setSize(winSize);
	operation->setSize(winSize);

	bgm = Sound::ins()->loadWave("Resources/BGM/mmc_140_BGM1.wav");
	transitionSe = Sound::ins()->loadWave("Resources/SE/Shortbridge29-1.wav");

	transitionTimer = std::make_unique<Timer>();
	updateProc = std::bind(&TitleScene::update_main, this);
}

TitleScene::~TitleScene()
{}

void TitleScene::start()
{
	Sound::playWave(bgm, XAUDIO2_LOOP_INFINITE, 0.2f);
}

void TitleScene::update_main()
{
	if (PadImu::ins()->checkInputAccept())
	{
		operation->isInvisible = false;
		updateProc = std::bind(&TitleScene::update_operation, this);
	} else
	{
		// Rキーでステージをリセット
		if (Input::ins()->triggerKey(DIK_R))
		{
			// JoyShockLibraryのパッド接続状況をリセット
			PadImu::ins()->reset();

			PlayScene::resetStageNum();
		}
	}
}

void TitleScene::update_operation()
{
	// 一秒で動くピクセル数
	constexpr float operSpeed = endPos * 3.f;
	operation->position.y = std::max(0.f, operation->position.y - operSpeed / DX12Base::ins()->getFPS());

	if (PadImu::ins()->checkInputAccept())
	{
		Sound::stopWave(bgm);
		Sound::playWave(transitionSe, 0u, 0.2f);

		nowLoading->isInvisible = false;
		thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<StageSelectScene>(); });
		updateProc = std::bind(&TitleScene::update_end, this);

		transitionTimer->reset();
	} else
	{
		if (Input::ins()->triggerKey(DIK_R))
		{
			operation->isInvisible = true;
			operation->position.y = endPos;
			updateProc = std::bind(&TitleScene::update_main, this);
		}
	}
}

void TitleScene::update_end()
{
	const auto nowTime = transitionTimer->getNowTime();
	const float rate = static_cast<float>(nowTime) / static_cast<float>(transitionTime);

	logoSprite->position.y = std::lerp(0.f, endPos, Util::easeOutBounce(rate));
	backSprite->position.y = logoSprite->position.y;
	operation->position.y = logoSprite->position.y;

	if (nowTime >= transitionTime)
	{
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
	}
}

void TitleScene::update()
{
	updateProc();
}

void TitleScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
	nowLoading->drawWithUpdate(DX12Base::ins(), spBase.get());
	backSprite->drawWithUpdate(DX12Base::ins(), spBase.get());
	logoSprite->drawWithUpdate(DX12Base::ins(), spBase.get());
	operation->drawWithUpdate(DX12Base::ins(), spBase.get());

	if (nowLoading->isInvisible)
	{
		ImGui::Begin("pressSpace", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
		ImGui::PushFont(DX12Base::ins()->getBigImFont());
		ImGui::Text("Press Space...");
		ImGui::PopFont();
		ImGui::End();
	}
}