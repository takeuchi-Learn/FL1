#pragma once

#include <System/GameScene.h>
#include <Util/Stopwatch.h>
#include <GameCamera.h>
#include <Collision/CollisionShape.h>
#include <memory>
#include <functional>
#include <array>

class Camera;
class Light;
class ObjModel;
class Object3d;
class Sprite;
class SpriteBase;
class SoundData;
class ParticleMgr;
class Collision;
class Player;
class Goal;
class BackGround;
class GameMap;
class TutorialTexture;

class PlayScene :
	public GameScene
{
private:

	//std::unique_ptr<Camera> camera;
	std::unique_ptr<Light> light;
	std::unique_ptr<GameCamera> camera;

	std::unique_ptr<SpriteBase> spriteBase;
	std::array<std::unique_ptr<Sprite>, 2u> cinemaScope;

	std::unique_ptr<ParticleMgr> particle;

	std::unique_ptr<Player> player;
	std::unique_ptr<Goal> goal;

	std::unique_ptr<BackGround> backGround;
	std::unique_ptr<TutorialTexture> tutorialTexture;
	std::unique_ptr<GameMap> gameMap;

	std::unique_ptr<Stopwatch> timer;

	std::function<void()> updateProc;
	std::function<void()> updateCinemaScopeProc;

	std::weak_ptr<SoundData> bgm;

	static uint16_t stageNum;

	uint16_t gameOverDelayFrame = 0ui16;
	static constexpr uint16_t GAME_OVER_DELAY_FRAME = 60ui16;

private:
	/// @brief 衝突確認関数
	void checkCollision();
	bool checkMinMax(const CollisionShape::AABB& aabb);

	void update_start();
	void update_appearance();
	void update_main();
	void update_clear();

public:
	PlayScene();
	~PlayScene();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	static inline void resetStageNum() { stageNum = 0; }
	static inline void setStageNum(uint16_t num) { stageNum = num; }
	static inline uint16_t getStageNum() { return stageNum; }
};
