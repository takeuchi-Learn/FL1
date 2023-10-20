#include "Player.h"
#include <Camera/CameraObj.h>
#include <cmath>
#include <Input/Input.h>
#include <Util/Timer.h>
#include <fstream>
#include <Util/YamlLoader.h>
#include <3D/Light/Light.h>
using namespace DirectX;

bool Player::loadYamlFile()
{
	constexpr const char filePath[] = "Resources/DataFile/player.yml";

	Yaml::Node root{};
	YamlLoader::LoadYamlFile(root, filePath);

	auto* obj = gameObj->getObj();

	auto& startPos = obj->position;
	LoadYamlDataToFloat3(root, startPos);

	return false;
}

Player::Player(GameCamera* camera, ObjModel* model) :
	gameObj(std::make_unique<BaseGameObjectHavingHp>(camera, model))
	, gameCamera(camera)
{
	loadYamlFile();

	gameObj->setScale(XMFLOAT3(100.f,100.f,100.f));
	
	// 追従させるためにポインタを渡す
	//gameCamera->setParentObj(gameObj.get());
}

void Player::update()
{
	gameObj->update();

	move();
	jump();
}

void Player::draw()
{
	gameObj->draw(light);
}

void Player::jump()
{

}

void Player::move()
{
	// 角度を取得
	const float angle = gameCamera->getAngle();

	// 座標を取得
	XMFLOAT3 position = gameObj->getPosition();

	// 角度に応じて移動
	// 仮に動かす座標を設定
	const float addPos = angle * 0.001f;
	position.x += addPos;

	// 計算後セット
	gameObj->setPosition(position);


	
}
