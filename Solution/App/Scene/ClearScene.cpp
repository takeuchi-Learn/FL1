#include "TitleScene.h"
#include <2D/Sprite.h>
#include <2D/DebugText.h>
#include <Input/Input.h>
#include <System/SceneManager.h>
#include "ClearScene.h"

ClearScene::ClearScene() :
	spriteBase(std::make_unique<SpriteBase>())
{
	debugText = std::make_unique<DebugText>(spriteBase->loadTexture(L"Resources/debugfont.png"),
											spriteBase.get());
}

ClearScene::~ClearScene()
{}

void ClearScene::update()
{
	debugText->Print(Input::ins()->hitKey(DIK_SPACE) ? "Clear Scene\nHIT SPACE" : "Clear Scene",
					 0.f, 0.f);
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
	}
}

void ClearScene::drawFrontSprite()
{
	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	debugText->DrawAll();
}