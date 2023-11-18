/*****************************************************************//**
 * \file   TitleScene.h
 * \brief  タイトル画面
 *********************************************************************/

#pragma once
#include "System/GameScene.h"
#include <memory>
#include <thread>
#include <functional>

class Timer;

/// @brief タイトル画面シーンのクラス
class TitleScene :
	public GameScene
{
	std::unique_ptr<GameScene> nextScene = nullptr;
	std::unique_ptr<std::jthread> thread{};

	std::function<void()> updateProc{};
	std::unique_ptr<Timer> transitionTimer;

	std::string debugStr{};

	void update_main();
	void update_end();

public:
	TitleScene();
	~TitleScene();
	void update() override;
	void drawFrontSprite() override;
};