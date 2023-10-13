#pragma once
#include"System/GameScene.h"
#include<memory>
class Sprite;
class SpriteBase;
class GameOver:
	public GameScene
{
	std::unique_ptr<Sprite> gameOver;

	std::unique_ptr<SpriteBase> spritebase;
public:
	GameOver();
	~GameOver();
public:
	void update()override;
	void drawFrontSprite()override;
};

