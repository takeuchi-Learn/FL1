#include "StageSelectScene.h"
#include "PlayScene.h"
#include <System/Dx12Base.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <filesystem>
#include <format>

namespace
{
	bool stageExist(uint16_t stageNum)
	{
		const auto mapYamlPath = "Resources/Map/map_" + std::to_string(stageNum) + ".yml";
		return std::filesystem::exists(mapYamlPath);
	}
}

StageSelectScene::StageSelectScene()
{
	for (uint16_t i = 0u; i < UINT16_MAX; ++i)
	{
		if (!stageExist(i))
		{
			stageMaxNum = i == 0u ? 0u : i - 1u;
			break;
		}
	}
}

void StageSelectScene::update()
{
	if (Input::ins()->triggerKey(DIK_RIGHT))
	{
		if (currentStage < stageMaxNum) { ++currentStage; }
	} else if (Input::ins()->triggerKey(DIK_LEFT))
	{
		if (currentStage > 0u) { --currentStage; }
	} else if (Input::ins()->triggerKey(DIK_SPACE))
	{
		PlayScene::setStageNum(currentStage);
		SceneManager::ins()->changeScene<PlayScene>();
	}
}

void StageSelectScene::drawFrontSprite()
{
	using namespace ImGui;
	Begin("StageSelectScene::drawFrontSprite", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
	Text(std::format("currentStage: {}", currentStage).c_str());
	End();
}
