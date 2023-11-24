/*****************************************************************//**
 * \file   ClearScene.h
 * \brief  タイトル画面
 *********************************************************************/

#pragma once
#include "System/GameScene.h"
#include <memory>
#include <thread>
#include <functional>

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

	std::unique_ptr<GameScene> nextScene = nullptr;
	std::unique_ptr<std::jthread> thread{};

	bool checkInputOfStartTransition();

public:
	ClearScene();
	~ClearScene();
	void start() override;
	void update() override;
	void drawFrontSprite() override;
};