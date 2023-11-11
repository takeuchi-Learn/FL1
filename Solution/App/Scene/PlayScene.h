#pragma once

#include <System/GameScene.h>
#include <Util/Stopwatch.h>
#include <memory>

#include"../GameCamera.h"

class Sprite;
class SpriteBase;
class ParticleMgr;
class Player;
class Goal;
class Collision;
class GameMap;

class PlayScene :
	public GameScene
{
private:
	std::unique_ptr<GameCamera> camera;

	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<Sprite> sprite;

	std::unique_ptr<Player> player;

	std::unique_ptr<Goal> goal;

	std::unique_ptr<GameMap> gameMap;

private:
	/// @brief 衝突確認関数
	void checkCollision();

public:
	PlayScene();
	~PlayScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
