#pragma once

#include <System/GameScene.h>
#include <memory>

class GameCamera;
class GameMap;
class Player;
class Light;

class PlayScene :
	public GameScene
{
private:
	std::unique_ptr<GameCamera> camera;
	std::unique_ptr<Player> player;
	std::unique_ptr<GameMap> gameMap;

public:
	PlayScene();
	~PlayScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
