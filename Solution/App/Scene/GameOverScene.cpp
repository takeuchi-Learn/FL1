#include "GameOverScene.h"
#include <2D/Sprite.h>
#include <2D/DebugText.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include <PadImu.h>
#include <JoyShockLibrary.h>

#include "TitleScene.h"

GameOverScene::GameOverScene() :
	spriteBase(std::make_unique<SpriteBase>())
{
	debugText = std::make_unique<DebugText>(spriteBase->loadTexture(L"Resources/debugfont.png"),
											spriteBase.get());
	thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<TitleScene>(); });
}

GameOverScene::~GameOverScene()
{}

void GameOverScene::start()
{}

void GameOverScene::update()
{
	debugText->Print("GameOver Scene\nHIT SPACE", 0.f, 0.f);

	if (thread->joinable() &&
		checkInputOfStartTransition())
	{
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
	}
}

bool GameOverScene::checkInputOfStartTransition()
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

void GameOverScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	debugText->DrawAll();
}