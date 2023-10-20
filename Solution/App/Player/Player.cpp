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
	gameCamera->setParentObj(gameObj.get());
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

void Player::updateJumpPos()
{
	fallTime++;

	const float PRE_VEL_Y = currentFallVelovity;
	currentFallVelovity = calcFallVelocity(fallStartSpeed, gAcc, fallTime);
	const float ADD_VEL_Y = currentFallVelovity - PRE_VEL_Y;


	//毎フレーム速度を加算
	XMFLOAT3 position = gameObj->getPosition();
	position.y += currentFallVelovity;
	gameObj->setPosition(position);
}

void Player::jump()
{
	// 一旦バウンド中はジャンプ禁止
	if (isBound)return;


	// ジャンプ時のカメラの追従をどうするか聞く
	// 完全に追従するかそうじゃないか

	// ここジャイロ実装したら書き換える
	if (Input::getInstance()->triggerKey(DIK_Z))
	{
		isJump = true;

		// 初速度などを設定
		// ジャイロの値を取得できるようになったらここをジャイロの数値を適当に変換して代入する
		fallStartSpeed = 18.f;
	}

	if (!isJump)return;

	// ジャンプの更新処理
	updateJumpPos();

	// 終了確認
	checkJumpEnd();
}

void Player::checkJumpEnd()
{
	if (isBound)return;

	XMFLOAT3 pos = gameObj->getPosition();
	// 仮に0.f以下になったらジャンプ終了
	if (pos.y < 0.f)
	{
		// ジャンプ終了処理
		isJump = false;
		fallTime = 0;
		pos.y = 0.f;
		gameObj->setPosition(pos);

		// バウンド開始処理
		isBound = true;
		fallStartSpeed = 5.f;
	}

}

void Player::bound()
{
	if (!isBound)return;

	// 更新
	updateJumpPos();

	// 終了確認
	checkBoundEnd();
}

void Player::checkBoundEnd()
{
	XMFLOAT3 pos = gameObj->getPosition();
	// 仮に0.f以下になったらジャンプ終了
	if (pos.y < 0.f)
	{
		isBound = false;
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
	rot();
}

void Player::rot()
{
	// 角度取得
	const float angleZ = gameObj->getRotation().z;
	const float cameraAngle = gameCamera->getAngle();
	// 角度計算
	float setAngle = -cameraAngle * 0.5f + angleZ;

	// オーバーフロー対策
	if (angleZ <= -360.f)
	{
		// 360足す
		setAngle += 360.f;
	}

	// セット
	gameObj->setRotationZ(setAngle);
}
