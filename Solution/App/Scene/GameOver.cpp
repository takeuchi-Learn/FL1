#include "GameOver.h"
#include<Input/Input.h>
#include<System/SceneManager.h>
#include"TitleScene.h"
#include<2D/Sprite.h>
#include<2D/SpriteBase.h>
using namespace DirectX;

GameOver::GameOver()
{
	spritebase = std::make_unique<SpriteBase>();
	gameOver = std::make_unique<Sprite>(spritebase->loadTexture(L"Resources/GameOver.png"),
										 spritebase.get(), XMFLOAT2(0, 0));
}
GameOver::~GameOver()
{}
void GameOver::update()
{
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
	}
}

void GameOver::drawFrontSprite()
{
	spritebase->drawStart(DX12Base::ins()->getCmdList());
	gameOver->drawWithUpdate(DX12Base::ins(), spritebase.get());
}
