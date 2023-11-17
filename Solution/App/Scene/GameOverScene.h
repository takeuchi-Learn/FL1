/*****************************************************************//**
 * \file   TitleScene.h
 * \brief  タイトル画面
 *********************************************************************/

#pragma once
#include "System/GameScene.h"
#include <memory>
#include <thread>

class Sprite;
class SpriteBase;
class DebugText;

/// @brief タイトル画面シーンのクラス
class GameOverScene :
	public GameScene
{
	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spriteBase;

	// --------------------
	// デバッグテキスト
	// --------------------
	std::unique_ptr<DebugText> debugText;

	std::unique_ptr<GameScene> nextScene = nullptr;
	std::unique_ptr<std::jthread> thread{};

public:
	GameOverScene();
	~GameOverScene();
	void update() override;
	void drawFrontSprite() override;
};