#pragma once

#include <System/GameScene.h>
#include <memory>

#include"../GameCamera.h"

class Camera;
class Light;
class ObjModel;
class Object3d;
class Sprite;
class SpriteBase;
class SoundData;
class ParticleMgr;

class Player;
class Light;

class PlayScene :
	public GameScene
{
private:
	std::unique_ptr<GameCamera> camera;
	std::unique_ptr<Player> player;

	std::unique_ptr<Goal> goal;

private:
	/// @brief 衝突確認関数
	void checkCollision();
	std::unique_ptr<GameMap> gameMap;

public:
	PlayScene();
	~PlayScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
