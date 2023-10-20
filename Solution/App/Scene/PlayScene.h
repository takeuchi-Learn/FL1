#pragma once

#include <System/GameScene.h>
#include <memory>

class Light;
class GameCamera;
class Billboard;
class BillboardData;

class PlayScene :
	public GameScene
{
private:
	std::unique_ptr<Light> light;
	std::unique_ptr<GameCamera> camera;

	std::unique_ptr<Billboard> billboards;

public:
	PlayScene();
	~PlayScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
