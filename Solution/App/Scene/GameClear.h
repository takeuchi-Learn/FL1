#pragma once
#include"System/GameScene.h"
#include<memory>
class Sprite;
class SpriteBase;
class GameClear :
	public GameScene
{
	std::unique_ptr<Sprite> gameClaer;

	std::unique_ptr<SpriteBase> spritebase;
public:
	GameClear();
	~GameClear();

	void update()override;
	void drawFrontSprite()override;
};

