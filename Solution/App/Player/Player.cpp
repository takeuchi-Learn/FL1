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

	constexpr float scale = 50.0f;
	gameObj->setScale(XMFLOAT3(scale, scale, scale));
	
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
	// ジャンプ時のカメラの追従をどうするか聞く
	// 完全に追従するかそうじゃないか

	if (Input::getInstance()->triggerKey(DIK_Z))
	{
		isJump = true;

		// 初速度などを設定
		// ジャイロの値を取得できるようになったらここをジャイロの数値を適当に変換して代入する
		fallStartSpeed = 18.f;
	}

	if (isJump)
	{
		// 重力加速度
		// 大きくすればするほど落下が早くなる
		constexpr float gAcc = 0.35f;

		fallTime++;

		const float PRE_VEL_Y = currentFallVelovity;
		currentFallVelovity = calcFallVelocity(fallStartSpeed, gAcc, fallTime);
		const float ADD_VEL_Y = currentFallVelovity - PRE_VEL_Y;


		//毎フレーム速度を加算
		XMFLOAT3 position = gameObj->getPosition();
		position.y += currentFallVelovity;
		gameObj->setPosition(position);
	}

	checkJumpEnd();
}

void Player::checkJumpEnd()
{

	XMFLOAT3 pos = gameObj->getPosition();
	// 仮に0.f以下になったらジャンプ終了
	if (pos.y < 0.f)
	{
		isJump = false;
		fallTime = 0;
		pos.y = 0.f;
		gameObj->setPosition(pos);
	}

}

void Player::move()
{
	// 角度を取得
	const float angle = gameCamera->getAngle();

	// 座標を取得
	XMFLOAT3 position = gameObj->getPosition();

	// 角度に応じて移動
	// 一旦加速は考慮せずに実装
	// ここ変更すると速度が変わる
	constexpr float speedMag = 0.35f;

	const float addPos = angle * speedMag;
	position.x += addPos;

	// 計算後セット
	gameObj->setPosition(position);

	// 回転
	float cameraAngle = gameCamera->getAngle();
	gameObj->setRotationZ(-cameraAngle * 0.5f + gameObj->getRotation().z);
}
