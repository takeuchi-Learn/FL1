#include "ClearScene.h"
#include <2D/Sprite.h>
#include <2D/DebugText.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <Input/PadImu.h>

#include "TitleScene.h"

#include"PlayScene.h"

using namespace DirectX;

ClearScene::ClearScene() :
	spriteBase(std::make_unique<SpriteBase>())
{
	sprite = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/gameclear.png"), spriteBase.get(), XMFLOAT2(0.f, 0.f));
}

ClearScene::~ClearScene()
{}

void ClearScene::start()
{}

void ClearScene::update()
{
	if (checkInputOfStartTransition())
	{
		// todo ファイルがあるかどうかで決める
		constexpr unsigned short stageMax = 3;

		const unsigned short stageNum = PlayScene::getStageNum();
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
		const int preState = PadImu::ins()->getPreStates()[0].buttons;
		const int state = PadImu::ins()->getStates()[0].buttons;

		const bool pre = PadImu::hitButtons(preState, useJSLMask);
		const bool current = PadImu::hitButtons(state, useJSLMask);

		if (!pre && current) { return true; }
	}

	return false;
}

void ClearScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	sprite->drawWithUpdate(DX12Base::ins(), spriteBase.get());

	ImGui::Begin("pressSpace", nullptr, DX12Base::imGuiWinFlagsNoTitleBar);
	ImGui::PushFont(DX12Base::ins()->getBigImFont());
	ImGui::Text("Press Space...");
	ImGui::PopFont();
	ImGui::End();
}