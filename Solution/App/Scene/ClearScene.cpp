﻿#include "ClearScene.h"
#include <2D/Sprite.h>
#include <2D/DebugText.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <Input/PadImu.h>
#include <Util/Timer.h>
#include <Util/Util.h>
#include <filesystem>

#include "TitleScene.h"
#include "PlayScene.h"

using namespace DirectX;

namespace
{
	constexpr Timer::timeType transitionTime = Timer::timeType(Timer::oneSecF * 1.5f);
}

ClearScene::ClearScene() :
	spriteBase(std::make_unique<SpriteBase>()),
	transitionTimer(std::make_unique<Timer>())
{
	updateProc = std::bind(&ClearScene::update_main, this);
	sprite = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/ステージクリア.png"), spriteBase.get(), XMFLOAT2(0.f, 0.f));
	spriteBack = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/BackSprite.png"), spriteBase.get(), XMFLOAT2(0.f, 0.f));

	nowLoading = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/nowLoading.png"), spriteBase.get(), XMFLOAT2(0.f, 0.f));
	nowLoading->isInvisible = true;
}

ClearScene::~ClearScene()
{}

void ClearScene::start()
{}

void ClearScene::update()
{
	updateProc();
}

bool ClearScene::checkInputOfStartTransition()
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
		const int preState = PadImu::ins()->getPreStates()[0].buttons;
		const int state = PadImu::ins()->getStates()[0].buttons;

		const bool pre = PadImu::hitButtons(preState, useJSLMask);
		const bool current = PadImu::hitButtons(state, useJSLMask);

		if (!pre && current) { return true; }
	}

	return false;
}

void ClearScene::update_main()
{
	if (checkInputOfStartTransition())
	{
		nowLoading->isInvisible = false;

		// 該当ステージがあればそれを始め、無ければタイトルへ戻る
		const auto mapYamlPath = "Resources/Map/map_" + std::to_string(PlayScene::getStageNum()) + ".yml";
		if (std::filesystem::exists(mapYamlPath))
		{
			thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<PlayScene>(); });
		} else
		{
			PlayScene::resetStageNum();
			thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<TitleScene>(); });
		}

		updateProc = std::bind(&ClearScene::update_end, this);
		transitionTimer->reset();
	}
}

void ClearScene::update_end()
{
	const auto nowTime = transitionTimer->getNowTime();
	const float rate = static_cast<float>(nowTime) / static_cast<float>(transitionTime);

	constexpr float endPos = static_cast<float>(WinAPI::window_height) + 10.f;
	sprite->position.y = std::lerp(0.f, endPos, Util::easeOutBounce(rate));
	spriteBack->position.y = std::lerp(0.f, endPos, Util::easeOutBounce(rate));

	if (thread->joinable()
		&& nowTime >= transitionTime)
	{
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
	}
}

void ClearScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	nowLoading->drawWithUpdate(DX12Base::ins(), spriteBase.get());
	spriteBack->drawWithUpdate(DX12Base::ins(), spriteBase.get());
	sprite->drawWithUpdate(DX12Base::ins(), spriteBase.get());
	if (nowLoading->isInvisible)
	{
		ImGui::Begin("pressSpace", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
		ImGui::PushFont(DX12Base::ins()->getBigImFont());
		ImGui::Text("Press Space...");
		ImGui::PopFont();
		ImGui::End();
	}
}