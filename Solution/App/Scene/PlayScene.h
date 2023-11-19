#pragma once

#include <System/GameScene.h>
#include <Util/Stopwatch.h>
#include <memory>
#include <functional>

#include "../GameCamera.h"

class Camera;
class Light;
class ObjModel;
class Object3d;
class Sprite;
class SpriteBase;
class SoundData;
class ParticleMgr;

class Player;
class Goal;
class BackGround;

class Collision;

class GameMap;

class PlayScene :
	public GameScene
{
private:

	//std::unique_ptr<Camera> camera;
	std::unique_ptr<GameCamera> camera;

	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<Sprite> sprite;


	std::unique_ptr<Player> player;

	std::unique_ptr<Goal> goal;

	std::unique_ptr<BackGround> backGround;
	std::unique_ptr<GameMap> gameMap;

	std::function<void()> updateProc;
	std::unique_ptr<Stopwatch> timer;

#pragma region ゲームオーバー関係
	int gameOverTimer = 0;
	const int GAME_OVER_TIME_MAX = (int)(60.f * 1.f);
#pragma endregion


private:
	/// @brief 衝突確認関数
	void checkCollision();

	void update_start();
	void update_main();
	
public:
	PlayScene();
	~PlayScene();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
