/*****************************************************************//**
 * \file   StageSelectScene.h
 * \brief  ステージ選択画面
 *********************************************************************/

#pragma once
#include <System/GameScene.h>
#include <memory>

 /// @brief ステージ選択画面のクラス
class StageSelectScene :
	public GameScene
{
	uint16_t  currentStage = 0;
	uint16_t stageMaxNum = 0;
public:
	StageSelectScene();
	~StageSelectScene() = default;

	void update() override;
	void drawFrontSprite() override;
};