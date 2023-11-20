#include "PlayScene.h"
#include <System/DX12Base.h>
#include <System/SceneManager.h>
#include <Input/Input.h>
#include <Camera/Camera.h>
#include <3D/Light/Light.h>
#include <3D/Obj/Object3d.h>
#include <3D/Obj/ObjModel.h>
#include <2D/Sprite.h>
#include <Sound/Sound.h>
#include <Sound/SoundData.h>
#include <3D/ParticleMgr.h>
#include <algorithm>

#include <Player/Player.h>
#include <Object/Goal.h>
#include <BackGround.h>
#include <GameMap.h>

#include <Collision/Collision.h>

#include "TitleScene.h"

using namespace DirectX;

namespace
{
	constexpr auto bgmPath = "Resources/BGM.wav";
	constexpr auto particleGraphPath = L"Resources/judgeRange.png";
	constexpr auto billboardGraphPath = L"Resources/judgeRange.png";
	constexpr auto mapYamlPath = "Resources/Map/map.yml";

	constexpr XMFLOAT3 objectPosDef = XMFLOAT3(0, 0, 0);
	constexpr XMFLOAT3 cameraPosDef = XMFLOAT3(0, 0, -600);
}

void PlayScene::checkCollision()
{
	const auto& mapAABBs = gameMap->getAABBs();
	for(auto y = 0; y < mapAABBs.size();y++)
	{
		for (auto x = 0; x < mapAABBs[y].size(); x++)
		{
			if (!checkMinMax(mapAABBs[y][x]))continue;

			if(Collision::CheckSphere2AABB(player->getShape(), mapAABBs[y][x]))
			{
				player->hit(mapAABBs[y][x], typeid(*gameMap).name());
			}
		}
	}

	// テストです
	//bool res = Collision::CheckSphere2AABB(player->getShape(), goal->getShape());
	//if (res)
	//{
	//	//player->hit();
	//}

}

bool PlayScene::checkMinMax(const CollisionShape::AABB& aabb)
{
	XMFLOAT2 minPos(aabb.minPos.m128_f32[0], aabb.minPos.m128_f32[1]);
	XMFLOAT2 maxPos(aabb.maxPos.m128_f32[0], aabb.maxPos.m128_f32[1]);
	if (minPos.x == 0.f && minPos.y == 0.f && maxPos.x == 0.f && maxPos.y == 0.f)return false;
	return true;
}

PlayScene::PlayScene() :
	camera(std::make_unique<GameCamera>())
{
	spriteBase = std::make_unique<SpriteBase>();
	sprite = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/judgeRange.png"),
									  spriteBase.get(),
									  XMFLOAT2(0.5f, 0.5f));
	sprite->color.w = 0.5f;

	camera->setEye(XMFLOAT3(0, 0, -5));
	camera->setTarget(XMFLOAT3(0, 0, 0));
	camera->setPerspectiveProjFlag(false);

	player = std::make_unique<Player>(camera.get());
	// 追従させるためにポインタを渡す
	camera->setParentObj(player->getObj().get());

	// 仮にプレイヤーモデルを割り当て
	//constexpr DirectX::XMFLOAT2 goalPos(80, 0);
	//goal = std::make_unique<Goal>(nullptr, camera.get(),  goalPos);

	backGround = std::make_unique<BackGround>(camera.get());

	player = std::make_unique<Player>(camera.get());
	
	// ゲームオーバー扱いになる座標をセット(セットした値をプレイヤーの座標が下回ったら落下死)
	player->setGameOverPos(gameMap->getGameoverPos());

	sensor = Sensor::create();
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);

	sensor = Sensor::create();
	sensor->erase();
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);

	sensor = Sensor::create();
	player = std::make_unique<Player>(camera.get(), playerModel.get());
	player->setLight(light.get());

	Sound::ins()->playWave(bgm,
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);
	backGround->update();
	gameMap->update();
	player->update();

	player = std::make_unique<Player>(camera.get(), playerModel.get());
	player->setLight(light.get());

	Sound::ins()->playWave(bgm,
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);


	// ゲームオーバー確認
	if(player->getIsDead())
	{
		camera->changeStateGameover();
	}
=========
	gameMap = std::make_unique<GameMap>(camera.get());
	const bool ret = gameMap->loadDataFile(mapYamlPath);
	assert(false == ret);
>>>>>>>>> Temporary merge branch 2
}

PlayScene::~PlayScene()
{
<<<<<<<<< Temporary merge branch 1
	Sound::ins()->stopWave(bgm);
{}	gameMap->draw();
	player->draw();
	//goal->draw();
}

void PlayScene::drawFrontSprite()
{
	const auto mousePos = Input::ins()->calcMousePosFromSystem();
	sprite->position.x = mousePos.x;
	sprite->position.y = mousePos.y;

	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	sprite->drawWithUpdate(DX12Base::ins(), spriteBase.get());

	using namespace ImGui;

	SetNextWindowPos(ImVec2(0.f, 0.f));

	Begin("PlaySceneGUI", nullptr, DX12Base::imGuiWinFlagsDef);
	Text("スペース: シーン切り替え");
	Text("x:%.1f, y:%.1f, z:%.1f",
		camera->getEye().x,
		camera->getEye().y,
		camera->getEye().z);
	Text("Accel : x:%.1f, y:%.1f, z:%.1f",
		 camera->getSensor()->GetAccelX(),
		 camera->getSensor()->GetAccelY(),
		 camera->getSensor()->GetAccelZ());
	Text("Gyro : x:%.1f, y:%.1f, z:%.1f",
		 camera->getSensor()->GetGyroX(),
		 camera->getSensor()->GetGyroY(),
		 camera->getSensor()->GetGyroZ());
	Text("Angle : %.1f",
		 camera->getAngle());
	End();
}