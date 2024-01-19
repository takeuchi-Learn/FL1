/*****************************************************************//**
 * \file   StageSelectScene.h
 * \brief  ステージ選択画面
 *********************************************************************/

#pragma once
#include <System/GameScene.h>
#include <memory>
#include <thread>
#include <functional>
#include <string>

class Stopwatch;

/// @brief ステージ選択画面のクラス
class StageSelectScene :
	public GameScene
{
	uint16_t  currentStage = 0;
	uint16_t stageMaxNum = 0;

	std::unique_ptr<std::jthread> thread{};
	std::unique_ptr<GameScene> nextScene = nullptr;
	std::function<void()> update_proc{};
	std::unique_ptr<Stopwatch> timer;
	float transitionRaito = 0.f;

	void update_main();
	void update_transition();

public:
	StageSelectScene();
	~StageSelectScene() = default;

	void update() override;
	void drawFrontSprite() override;
};