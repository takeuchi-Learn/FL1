#include "GameOverScene.h"
#include "StageSelectScene.h"
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <2D/Sprite.h>
#include <2D/SpriteBase.h>
#include <Util/Util.h>
#include <Sound/Sound.h>
#include <Util/Timer.h>
#include <algorithm>
#include <Input/PadImu.h>

using namespace DirectX;

namespace
{
	constexpr Timer::timeType transitionTime = Timer::timeType(Timer::oneSecF * 1.5f);
}

GameOverScene::GameOverScene() :
	transitionTimer(std::make_unique<Timer>())
{
	spBase = std::make_unique<SpriteBase>();
	sprite = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/gameover.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	nowLoading = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/nowLoading.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	nowLoading->isInvisible = true;

	bgm = Sound::ins()->loadWave("Resources/BGM/BGM.wav");
	transitionSe = Sound::ins()->loadWave("Resources/SE/Shortbridge29-1.wav");

	updateProc = std::bind(&GameOverScene::update_main, this);
	thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<StageSelectScene>(); });
}

GameOverScene::~GameOverScene()
{}

void GameOverScene::start()
{
	Sound::playWave(bgm, XAUDIO2_LOOP_INFINITE, 0.4f);
}

void GameOverScene::update()
{
	updateProc();
}

void GameOverScene::update_main()
{
	if (PadImu::ins()->checkInputAccept())
	{
		nowLoading->isInvisible = false;

		Sound::stopWave(bgm);
		Sound::playWave(transitionSe, 0u, 0.2f);
		updateProc = std::bind(&GameOverScene::update_end, this);
		transitionTimer->reset();
	}
}

void GameOverScene::update_end()
{
	const auto nowTime = transitionTimer->getNowTime();
	const float rate = static_cast<float>(nowTime) / static_cast<float>(transitionTime);

	constexpr float endPos = static_cast<float>(WinAPI::window_height) + 10.f;
	sprite->position.y = std::lerp(0.f, endPos, Util::easeOutBounce(rate));

	if (thread->joinable()
		&& nowTime >= transitionTime)
	{
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
	}
}

void GameOverScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
	nowLoading->drawWithUpdate(DX12Base::ins(), spBase.get());
	sprite->drawWithUpdate(DX12Base::ins(), spBase.get());

	if (nowLoading->isInvisible)
	{
		ImGui::Begin("pressSpace", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
		ImGui::PushFont(DX12Base::ins()->getBigImFont());
		ImGui::Text("Press Space...");
		ImGui::PopFont();
		ImGui::End();
	}
}