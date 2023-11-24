#include "TitleScene.h"
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <Util/Timer.h>
#include <PadImu.h>
#include <2D/Sprite.h>
#include <2D/SpriteBase.h>
#include <Util/Util.h>
#include <Sound/Sound.h>
#include <algorithm>
#include <JoyShockLibrary.h>

#include "PlayScene.h"

using namespace DirectX;

namespace
{
	constexpr float transitionTimeSec = 1.25f;
	constexpr auto transitionTime = static_cast<Timer::timeType>(Timer::oneSecF * transitionTimeSec);
}

TitleScene::TitleScene()
{
	spBase = std::make_unique<SpriteBase>();
	titleSprite = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/title.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	nowLoading = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/nowLoading.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	nowLoading->isInvisible = true;

	bgm = Sound::ins()->loadWave("Resources/BGM/mmc_140_BGM1.wav");
	transitionSe = Sound::ins()->loadWave("Resources/SE/Shortbridge29-1.wav");

	transitionTimer = std::make_unique<Timer>();
	updateProc = std::bind(&TitleScene::update_main, this);
}

TitleScene::~TitleScene()
{}

void TitleScene::start()
{
	// JoyShockLibraryのパッド接続状況をリセット
	PadImu::ins()->reset();

	Sound::playWave(bgm, XAUDIO2_LOOP_INFINITE, 0.2f);
}

void TitleScene::update_main()
{
	if (checkInputOfStartTransition())
	{
		Sound::stopWave(bgm);
		Sound::playWave(transitionSe, 0u, 0.2f);

		nowLoading->isInvisible = false;
		thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<PlayScene>(); });
		updateProc = std::bind(&TitleScene::update_end, this);

		transitionTimer->reset();
	}
}

void TitleScene::update_end()
{
	const auto nowTime = transitionTimer->getNowTime();
	const float rate = static_cast<float>(nowTime) / static_cast<float>(transitionTime);

	titleSprite->position.y = std::lerp(0.f, static_cast<float>(WinAPI::window_height), Util::easeOutBounce(rate));

	if (nowTime >= transitionTime)
	{
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
	}
}

bool TitleScene::checkInputOfStartTransition()
{
	constexpr auto useKey = DIK_SPACE;
	constexpr auto useXInputButton = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B;
	constexpr auto useJSLMask = JSMASK_E | JSMASK_S;

	if (Input::ins()->triggerKey(useKey)) { return true; }

	if (Input::ins()->triggerPadButton(useXInputButton))
	{
		return true;
	}

	if (PadImu::ins()->getDevCount() > 0)
	{
		const auto state = JslGetSimpleState(PadImu::ins()->getHandles()[0]);
		// todo hitkey相当になっているのでトリガーに変更する
		return static_cast<bool>(state.buttons & (useJSLMask));
	}

	return false;
}

void TitleScene::update()
{
	updateProc();
}

void TitleScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
	nowLoading->drawWithUpdate(DX12Base::ins(), spBase.get());
	titleSprite->drawWithUpdate(DX12Base::ins(), spBase.get());
}