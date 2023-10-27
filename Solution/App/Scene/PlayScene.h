#pragma once

#include <System/GameScene.h>
#include <memory>

class GameCamera;
class GameMap;
class ObjModel;
class Player;
class Light;

class PlayScene :
	public GameScene
{
private:
	std::unique_ptr<GameCamera> camera;

	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<Player> player;

private:
	std::unique_ptr<GameMap> gameMap;

	// todo Playerをビルボードにしたら消す
	std::unique_ptr<Light> light;

public:
	PlayScene();
	~PlayScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
