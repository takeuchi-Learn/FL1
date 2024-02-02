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
class Sprite;
class SpriteBase;
class TutorialTexture;
// TutorialTexture用
class GameCamera;

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

	
	// ステージ選択の画面はImGuiのウィンドウの上にかぶせるように配置すれば問題なさそう
	
	// スプライト準備するやつ
	std::unique_ptr<SpriteBase> spBase;

	// 背景
	std::unique_ptr<Sprite>backGroundSprite;
	// ステージ画面
	std::vector<std::unique_ptr<Sprite>>stageTexSprite;
	// 矢印
	std::unique_ptr<Sprite>arrowSprite;

	// コントローラー回転してるやつ
	std::unique_ptr<TutorialTexture>tutorialTexture;

	std::unique_ptr<GameCamera>camera;
public:
	StageSelectScene();
	~StageSelectScene() = default;

	void update() override;
	void drawFrontSprite() override;
};