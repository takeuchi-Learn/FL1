#include "GameClear.h"
#include<Input/Input.h>
#include<System/SceneManager.h>
#include"TitleScene.h"
#include<2D/Sprite.h>
#include<2D/SpriteBase.h>

using namespace DirectX;

GameClear::GameClear()
{
	spritebase = std::make_unique<SpriteBase>();
	gameClaer = std::make_unique<Sprite>(spritebase->loadTexture(L"Resources/GameClear.png"), 
										 spritebase.get(), XMFLOAT2(0, 0));
}

GameClear::~GameClear()
{}

void GameClear::update()
{
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
	}
}

void GameClear::drawFrontSprite()
{
	spritebase->drawStart(DX12Base::ins()->getCmdList());
	gameClaer->drawWithUpdate(DX12Base::ins(), spritebase.get());
}
