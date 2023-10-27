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
	//// 絶対座標いじる前に書く
	//if (terrainHitObjPosX != 0.f)
	//{
	//	// 座標戻す
	//	gameObj->setPosition(XMFLOAT3(terrainHitPosX, 0.f, 0.f));

	//}

	gameObj->update();

	// ベクトル計測用
	preFramePos = currentFramePos;
	XMFLOAT3 objPos = gameObj->getPosition();
	currentFramePos = XMFLOAT2(objPos.x, objPos.y);

	move();
	jump();
	rebound();

	//// 一旦ここに書いてる
	//if(terrainHitObjPosX != 0.f)
	//{
	//	gameObj->setPosition(XMFLOAT3(terrainHitPosX, 0.f, 0.f));
	//}
}

void Player::draw()
{
	gameObj->draw(light);
}

void Player::calcJumpPos()
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
	if (isReboundY)return;

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
	calcJumpPos();

	// 終了確認
	checkJumpEnd();
}

void Player::checkJumpEnd()
{
	if (isReboundY)return;

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

	// 横バウンド時の座標計算
	// 一旦壁と隣接してるフレームを描画するために先に呼び出している
	calcSideRebound();

	// ここから関数化1

	// 仮に80に設定
	constexpr float testP = 80.f;
	// 本来は衝突時に呼び出す
	const XMFLOAT3 pos = gameObj->getPosition();

	if (pos.x >= testP)
	{
		// ここに衝突したときの座標格納する
		terrainHitPosX = testP;

		gameObj->setPosition(XMFLOAT3(80.f, pos.y, pos.z));

		const XMFLOAT3 clampPos = gameObj->getPosition();
		currentFramePos = XMFLOAT2(clampPos.x, clampPos.y);

		// デバッグ用
		const float vec = preFramePos.x - currentFramePos.x;

		// 横のバウンド開始
		startSideRebound();
	}

	// ここまで関数化1




	if (!isReboundY)return;

	// 更新
	calcJumpPos();
	// 終了確認
	checkreBoundEnd();
}

void Player::startRebound()
{
	// 跳ね返り開始処理
	isReboundY = true;

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
			isReboundY = false;
			isDrop = false;
		}
		else
		{
			startRebound();
		}
	}
}

void Player::calcSideRebound()
{
	if (!isReboundX)return;

	// 座標に加算
	XMFLOAT3 pos = gameObj->getPosition();
	pos.x += sideAddX;
	gameObj->setPosition(pos);


	// 加算値を加算または減算
	
	// 変化する値
	constexpr float changeValue = 1.0f;
	if (sideAddX > 0 )
	{
		sideAddX -= changeValue;

		// 負の値になったら演算終了
		if(sideAddX <= 0)
		{
			sideAddX = 0;
			isReboundX = false;
		}
	}
	else
	{
		sideAddX += changeValue;

		if (sideAddX >= 0)
		{
			sideAddX = 0;
			isReboundX = false;
		}
	}

}

void Player::startSideRebound()
{
	const float vec = preFramePos.x - currentFramePos.x;

	// 速度に合わせて代入
	constexpr float mag = 1.f;
	sideAddX = vec * mag;

	isReboundX = true;

	terrainHitObjPosX = gameObj->getPosition().x;
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

	constexpr float accMag = 0.015f;
	acc += addPos * accMag;
	position.x += acc;

	// 停止したらリセット
	// ここジャストじゃなくて一定範囲でもいいかも
	if (preFramePos.x == currentFramePos.x)acc = 0.f;

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
