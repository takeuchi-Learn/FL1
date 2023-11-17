#include "TitleScene.h"
#include <2D/Sprite.h>
#include <2D/DebugText.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include "GameOverScene.h"

GameOverScene::GameOverScene() :
	spriteBase(std::make_unique<SpriteBase>())
{
	debugText = std::make_unique<DebugText>(spriteBase->loadTexture(L"Resources/debugfont.png"),
											spriteBase.get());
	thread = std::make_unique<std::jthread>([&] { nextScene = std::make_unique<TitleScene>(); });
}

GameOverScene::~GameOverScene()
{}

void GameOverScene::update()
{
	debugText->Print(Input::ins()->hitKey(DIK_SPACE) ? "GameOver Scene" : "GameOver Scene\nHIT SPACE",
					 0.f, 0.f);

	if (Input::ins()->triggerKey(DIK_SPACE) &&
		thread->joinable())
	{
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
	}
}

void GameOverScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	debugText->DrawAll();
}