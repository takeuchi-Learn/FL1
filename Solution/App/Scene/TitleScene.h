﻿/*****************************************************************//**
 * \file   TitleScene.h
 * \brief  タイトル画面
 *********************************************************************/

#pragma once
#include <System/GameScene.h>
#include <memory>
#include <thread>
#include <functional>

class Timer;
class Sprite;
class SpriteBase;
class SoundData;

/// @brief タイトル画面シーンのクラス
class TitleScene :
	public GameScene
{
	std::unique_ptr<GameScene> nextScene = nullptr;
	std::unique_ptr<std::jthread> thread{};

	std::function<void()> updateProc{};
	std::unique_ptr<Timer> transitionTimer;

	std::unique_ptr<Sprite> backSprite;
	std::unique_ptr<Sprite> logoSprite;
	std::unique_ptr<Sprite> nowLoading;
	std::unique_ptr<Sprite> operation;
	std::unique_ptr<SpriteBase> spBase;

	std::weak_ptr<SoundData> bgm;
	std::weak_ptr<SoundData> transitionSe;

	void update_main();
	void update_operation();
	void update_end();

public:
	TitleScene();
	~TitleScene();
	void start() override;
	void update() override;
	void drawFrontSprite() override;
};