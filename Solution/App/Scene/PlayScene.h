#pragma once

#include <System/GameScene.h>
#include <Util/Stopwatch.h>
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

class PlayScene :
	public GameScene
{
private:
	std::unique_ptr<Stopwatch> stopwatch{};
	Timer::timeType stopwatchPlayTime;

	std::weak_ptr<SoundData> bgm;

	std::unique_ptr<Light> light;
	std::unique_ptr<GameCamera> camera;

	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<Sprite> sprite;

	std::unique_ptr<ObjModel> model;
	std::unique_ptr<Object3d> object;
	size_t pbrPP{};

	std::unique_ptr<ParticleMgr> particle;

public:
	PlayScene();
	~PlayScene();

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;
};
