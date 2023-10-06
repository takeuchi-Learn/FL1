#include "TitleScene.h"
#include <2D/Sprite.h>
#include <2D/DebugText.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include "PlayScene.h"

TitleScene::TitleScene() :
	spriteBase(std::make_unique<SpriteBase>())
{
	debugText = std::make_unique<DebugText>(spriteBase->loadTexture(L"Resources/debugfont.png"),
											spriteBase.get());
}

TitleScene::~TitleScene()
{}

void TitleScene::update()
{
	debugText->Print(Input::ins()->hitKey(DIK_SPACE) ? "Title Scene\nHIT SPACE" : "Title Scene",
					 0.f, 0.f);
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<PlayScene>();
	}
}

void TitleScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	debugText->DrawAll();
}