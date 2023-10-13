#pragma once

#include <System/GameScene.h>
#include <Util/Stopwatch.h>
#include <memory>

class Camera;
class Light;
class ObjModel;
class Object3d;
class Sprite;
class SpriteBase;
class SoundData;
class ParticleMgr;
class Player;

class PlayScene :
	public GameScene
{
private:
	std::unique_ptr<Stopwatch> stopwatch{};
	Timer::timeType stopwatchPlayTime;

	std::weak_ptr<SoundData> bgm;

	std::unique_ptr<Light> light;
	std::unique_ptr<Camera> camera;

	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<Sprite> sprite;

	std::unique_ptr<ParticleMgr> particle;

	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<Player> player;

public:
	PlayScene();
	~PlayScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
