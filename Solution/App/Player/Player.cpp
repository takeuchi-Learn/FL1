#include "Player.h"
#include <Camera/CameraObj.h>
#include <cmath>
#include <Input/Input.h>
#include <Util/Timer.h>
#include <fstream>
#include <Util/YamlLoader.h>
#include <GameObject/BaseGameObjectHavingHp.h>
#include <3D/Light/Light.h>
using namespace DirectX;

bool Player::loadYamlFile()
{
	constexpr const char filePath[] = "Resources/DataFile/player.yml";

	Yaml::Node root{};

	YamlLoader::LoadYamlFile(root, filePath);

	return false;
}

Player::Player(Camera* camera, ObjModel* model) :
	gameObj(std::make_unique<BaseGameObjectHavingHp>(camera, model))
{}

void Player::update()
{
	gameObj->update();
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

}
