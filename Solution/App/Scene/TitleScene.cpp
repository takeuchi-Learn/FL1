#include "TitleScene.h"
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <Util/Timer.h>
#include <algorithm>
#include <JoyShockLibrary.h>

#include "PlayScene.h"

using namespace DirectX;

namespace
{
	constexpr float transitionTimeSec = 1.5f;
	constexpr auto transitionTime = static_cast<Timer::timeType>(Timer::oneSecF * transitionTimeSec);
}

TitleScene::TitleScene() :
	debugStr("HIT SPACE")
{
	thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<PlayScene>(); });

	transitionTimer = std::make_unique<Timer>();
	updateProc = std::bind(&TitleScene::update_main, this);
}

TitleScene::~TitleScene()
{}

void TitleScene::start()
{
	devCount = JslConnectDevices();
	if (devCount > 0)
	{
		devHandles.resize(devCount, -1);
		JslGetConnectedDeviceHandles(devHandles.data(), devCount);
	}
}

void TitleScene::update_main()
{
	if (checkInputOfStartTransition())
	{
		updateProc = std::bind(&TitleScene::update_end, this);
		transitionTimer->reset();
	}
}

void TitleScene::update_end()
{
	const auto nowTime = transitionTimer->getNowTime();
	const float rate = static_cast<float>(nowTime) / static_cast<float>(transitionTime);

	// 進行度を右詰め三桁で表示
	debugStr = std::format("{:_>3.0f}%%", 100.f * std::clamp(rate, 0.f, 1.f));
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

	if (devCount > 0)
	{
		const auto state = JslGetSimpleState(devHandles[0]);
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
	using namespace ImGui;
	Begin("TitleScene::drawFrontSprite()", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
	PushFont(DX12Base::ins()->getBigImFont());
	Text("TitleScene");
	PopFont();
	Text(debugStr.c_str());
	End();
}