﻿/*****************************************************************//**
 * \file   ClearScene.h
 * \brief  タイトル画面
 *********************************************************************/

#pragma once
#include "System/GameScene.h"
#include <memory>

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

	// --------------------
	// デバッグテキスト
	// --------------------
	std::unique_ptr<DebugText> debugText;

public:
	ClearScene();
	~ClearScene();
	void update() override;
	void drawFrontSprite() override;
};