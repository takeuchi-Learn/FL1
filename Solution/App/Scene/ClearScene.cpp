﻿#include "ClearScene.h"
#include <2D/Sprite.h>
#include <2D/DebugText.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <PadImu.h>
#include <JoyShockLibrary.h>

#include "TitleScene.h"

#include"PlayScene.h"

ClearScene::ClearScene() :
	spriteBase(std::make_unique<SpriteBase>())
{
	debugText = std::make_unique<DebugText>(spriteBase->loadTexture(L"Resources/debugfont.png"),
											spriteBase.get());
}

ClearScene::~ClearScene()
{}

void ClearScene::start()
{}

void ClearScene::update()
{
	debugText->Print("Clear Scene\nHIT SPACE", 0.f, 0.f);
	if (checkInputOfStartTransition())
	{
		const unsigned short stageNum = PlayScene::getStageNum();
		constexpr unsigned short stageMax = 2;
		if (stageNum == stageMax)
		{
			PlayScene::resetStageNum();
			SceneManager::ins()->changeScene<TitleScene>();
		}
		else
		{
			SceneManager::ins()->changeScene<PlayScene>();
		}
	}
}

bool ClearScene::checkInputOfStartTransition()
{
	constexpr auto useKey = DIK_SPACE;
	constexpr auto useXInputButton = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B;
	constexpr auto useJSLMask = JSMASK_E | JSMASK_S;

	if (Input::ins()->triggerKey(useKey)) { return true; }

	if (Input::ins()->triggerPadButton(useXInputButton))
	{
		return true;
	}

	if (PadImu::ins()->getDevCount() > 0)
	{
		const auto state = JslGetSimpleState(PadImu::ins()->getHandles()[0]);
		// todo hitkey相当になっているのでトリガーに変更する
		return static_cast<bool>(state.buttons & (useJSLMask));
	}

	return false;
}

void ClearScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	debugText->DrawAll();
}