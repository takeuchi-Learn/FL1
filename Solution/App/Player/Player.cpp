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

	// ベクトル計測用
	preFramePos = currentFramePos;
	XMFLOAT3 objPos = gameObj->getPosition();
	currentFramePos = XMFLOAT2(objPos.x, objPos.y);

	move();
	jump();
	rebound();
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
	// センサーの値によってジャンプ量細かく変更するか聞く

	// ジャンプパワー
	constexpr float jumpPower = 18.f;
	constexpr float bigJumpPower = 25.f;

	// 一旦跳ね返り中はジャンプ禁止
	if (isRebound)return;

	calcDropVec();

	// ジャンプするのに必要なジャイロの値
	constexpr float jumpSensorValue = 1.f;
	constexpr float bigSensorJyroValue = 2.f;

	if (Input::getInstance()->triggerKey(DIK_Z) || sensorValue >= jumpSensorValue)
	{
		isJump = true;

		// 大ジャンプ
		if (Input::getInstance()->hitKey(DIK_X) || sensorValue >= bigSensorJyroValue)
		{
			fallStartSpeed = bigJumpPower;
		}
		else// 通常ジャンプ
		{
			// 初速度を設定
			// ジャイロの値を取得できるようになったらここをジャイロの数値を適当に変換して代入する
			fallStartSpeed = jumpPower;
		}
	}

	

	if (!isJump)return;

	// ジャンプの更新処理
	updateJumpPos();

	// 終了確認
	checkJumpEnd();
}

void Player::checkJumpEnd()
{
	if (isRebound)return;

	XMFLOAT3 pos = gameObj->getPosition();
	// 仮に0.f以下になったらジャンプ終了
	if (pos.y < 0.f)
	{
		// ジャンプ終了処理
		isJump = false;
		fallTime = 0;
		pos.y = 0.f;
		gameObj->setPosition(pos);

		isDrop = false;

		startRebound();
	}

}

void Player::calcDropVec()
{
	preFramePos.y = currentFramePos.y;
	currentFramePos.y = gameObj->getPosition().y;

	// preの方が大きい(落下開始)し始めたら代入
	if(currentFramePos.y > preFramePos.y && !isDrop)
	{
		dropStartY = currentFramePos.y;
		isDrop = true;
	}
}

void Player::rebound()
{
	if (!isRebound)return;

	// 更新
	updateJumpPos();

	// 終了確認
	checkreBoundEnd();


	// 横のバウンド

}

void Player::startRebound()
{
	// 跳ね返り開始処理
	isRebound = true;

	constexpr float fallVelMag = 0.4f;
	// 落下した高さに合わせて跳ね返り量を変える
	fallStartSpeed = -currentFallVelovity * fallVelMag;
}

void Player::checkreBoundEnd()
{
	XMFLOAT3 pos = gameObj->getPosition();
	// 仮に0.f以下になったらジャンプ終了
	if (pos.y < 0.f)
	{
		fallTime = 0;
		pos.y = 0.f;
		gameObj->setPosition(pos);
		constexpr float boundEndVel = 3.f;
		if (-currentFallVelovity <= boundEndVel)
		{
			isRebound = false;
			isDrop = false;
		}
		else
		{
			startRebound();
		}
	}
}

void Player::sideRebound()
{}


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
