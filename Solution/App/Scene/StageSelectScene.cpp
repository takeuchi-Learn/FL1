#include "StageSelectScene.h"
#include "PlayScene.h"
#include <System/Dx12Base.h>
#include <Input/Input.h>
#include <Input/PadImu.h>
#include <Imu/Sensor.h>
#include <System/SceneManager.h>
#include <Util/Stopwatch.h>
#include <Util/Util.h>
#include <filesystem>
#include <format>

namespace
{
	constexpr auto transitionTime = Timer::oneSec * 1.5;

	inline bool stageExist(uint16_t stageNum)
	{
		const auto mapYamlPath = "Resources/Map/map_" + std::to_string(stageNum) + ".yml";
		return std::filesystem::exists(mapYamlPath);
	}
}

StageSelectScene::StageSelectScene() :
	timer(std::make_unique<Stopwatch>()),
	drawText("Press SPACE")
{
	// ステージ存在確認
	for (uint16_t i = 0u; i < UINT16_MAX; ++i)
	{
		if (!stageExist(i))
		{
			stageMaxNum = i == 0u ? 0u : i - 1u;
			break;
		}
	}
	currentStage = std::clamp(PlayScene::getStageNum(), 0ui16, stageMaxNum);

	update_proc = std::bind(&StageSelectScene::update_main, this);
}

void StageSelectScene::update_main()
{
	bool inputR = Input::ins()->triggerKey(DIK_RIGHT);
	inputR |= Input::ins()->triggerPadButton(Input::PAD::PADNUM::RIGHT);

	bool inputL = Input::ins()->triggerKey(DIK_LEFT);
	inputL |= Input::ins()->triggerPadButton(Input::PAD::PADNUM::LEFT);

	if (PadImu::ins()->getDevCount() > 0)
	{
		inputR |= PadImu::ins()->checkTriggerInputDPAD(0, PadImu::DIRECTION::RIGHT);
		inputL |= PadImu::ins()->checkTriggerInputDPAD(0, PadImu::DIRECTION::LEFT);
	}

	if (inputR)
	{
		if (currentStage < stageMaxNum) { ++currentStage; }
	} else if (inputL)
	{
		if (currentStage > 0u) { --currentStage; }
	} else if (PadImu::ins()->checkInputAccept() || Sensor::ins()->CheckButton())
	{
		drawText = "NOW LOADING...";
		PlayScene::setStageNum(currentStage);
		thread = std::make_unique<std::jthread>([&]
												{
													nextScene = std::make_unique<PlayScene>();
												});
		update_proc = std::bind(&StageSelectScene::update_transition, this);
		timer->reset();
	}
}

void StageSelectScene::update_transition()
{
	const auto nowTime = timer->getNowTime();
	if (nowTime >= transitionTime)
	{
		transitionRaito = 1.f;
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
		return;
	}

	transitionRaito = static_cast<float>(nowTime) / static_cast<float>(transitionTime);
}

void StageSelectScene::update()
{
	update_proc();
}

void StageSelectScene::drawFrontSprite()
{
	using namespace ImGui;
	SetNextWindowPos(ImVec2(0, 0));
	Begin("StageSelectScene::drawFrontSprite", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
	Text(std::format("currentStage: {}", currentStage).c_str());
	Text(drawText.c_str());
	End();

	const auto stageCount = stageMaxNum + 1;
	for (int i = 0; i < stageCount; ++i)
	{
		// todo GameMapのstatic定数にする
		constexpr float mapSize = 100.f;
		constexpr float sizeMax = mapSize * 2.f;

		{
			const float shiftVal = mapSize * 1.5f;
			constexpr auto posY = static_cast<float>(WinAPI::window_height) / 2.f;
			constexpr auto posX = static_cast<float>(WinAPI::window_width) / 2.f;
			SetNextWindowPos(ImVec2(posX + (int(i) - int(currentStage)) * sizeMax, posY), 0, ImVec2(0.5f, 0.5f));
		}

		{
			float raito = static_cast<float>(stageCount - std::abs(int(currentStage) - int(i))) / static_cast<float>(stageCount);
			raito = 1.f - raito;
			raito = 1.f - raito * raito;
			raito = std::clamp(raito, 0.f, 1.f);

			float size = -1.f;
			if (i == currentStage)
			{
				constexpr float transitionEndSize = static_cast<float>(1 + std::max(WinAPI::window_width, WinAPI::window_height));
				size = std::lerp(sizeMax, transitionEndSize, Util::easeOutBounce(transitionRaito));
			} else
			{
				constexpr auto min = sizeMax * 0.125f;
				constexpr auto max = sizeMax * 0.875f;
				size = std::lerp(min, max, raito);
			}

			SetNextWindowSize(ImVec2(size, size));
		}

		Begin(std::format("StageSelectScene::drawFrontSprite{}", i).c_str(), nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
		Text(std::format("{}", i).c_str());
		End();
	}
}