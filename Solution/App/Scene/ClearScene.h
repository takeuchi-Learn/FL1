/*****************************************************************//**
 * \file   ClearScene.h
 * \brief  タイトル画面
 *********************************************************************/

#pragma once
#include "System/GameScene.h"
#include <memory>
#include <thread>
#include <functional>

class Timer;
class Sprite;
class SpriteBase;
class DebugText;

/// @brief タイトル画面シーンのクラス
class ClearScene :
	public GameScene
{
	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<Sprite> nowLoading;

	std::unique_ptr<GameScene> nextScene = nullptr;
	std::unique_ptr<std::jthread> thread{};

	std::function<void()> updateProc{};
	std::unique_ptr<Timer> transitionTimer;

	bool checkInputOfStartTransition();

	void update_main();
	void update_end();

public:
	ClearScene();
	~ClearScene();
	void start() override;
	void update() override;
	void drawFrontSprite() override;
};