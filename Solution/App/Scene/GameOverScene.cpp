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
}

GameOverScene::~GameOverScene()
{}

void GameOverScene::update()
{
	debugText->Print(Input::ins()->hitKey(DIK_SPACE) ? "GameOver Scene\nHIT SPACE" : "GameOver Scene",
					 0.f, 0.f);
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
	}
}

void GameOverScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	debugText->DrawAll();
}