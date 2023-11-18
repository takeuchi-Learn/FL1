#include "TitleScene.h"
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <Util/Timer.h>
#include <algorithm>

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

void TitleScene::update_main()
{
	if (Input::ins()->triggerKey(DIK_SPACE))
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