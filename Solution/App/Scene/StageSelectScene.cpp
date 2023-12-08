#include "StageSelectScene.h"
#include "PlayScene.h"
#include <System/Dx12Base.h>
#include <Input/Input.h>
#include <Input/PadImu.h>
#include <System/SceneManager.h>
#include <Util/Stopwatch.h>
#include <filesystem>
#include <format>

namespace
{
	constexpr auto transitionTime = Timer::oneSec;

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

	update_proc = std::bind(&StageSelectScene::update_main, this);
}

void StageSelectScene::update_main()
{
	bool inputR = Input::ins()->triggerKey(DIK_RIGHT);
	inputR |= Input::ins()->triggerPadButton(Input::PAD::PADNUM::RIGHT);
	inputR |= PadImu::ins()->checkTriggerInputDPAD(0, PadImu::DIRECTION::RIGHT);

	bool inputL = Input::ins()->triggerKey(DIK_LEFT);
	inputL |= Input::ins()->triggerPadButton(Input::PAD::PADNUM::LEFT);
	inputL |= PadImu::ins()->checkTriggerInputDPAD(0, PadImu::DIRECTION::LEFT);

	if (inputR)
	{
		if (currentStage < stageMaxNum) { ++currentStage; }
	} else if (inputL)
	{
		if (currentStage > 0u) { --currentStage; }
	} else if (PadImu::ins()->checkInputAccept())
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
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
		return;
	}
}

void StageSelectScene::update()
{
	update_proc();
}

void StageSelectScene::drawFrontSprite()
{
	using namespace ImGui;
	Begin("StageSelectScene::drawFrontSprite", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
	Text(std::format("currentStage: {}", currentStage).c_str());
	Text(drawText.c_str());
	End();
}
