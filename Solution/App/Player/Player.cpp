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

	XMFLOAT2& startPos = mapPos;
	LoadYamlDataToFloat2(root, startPos);

	return false;
}

Player::Player(GameCamera* camera) :
	gameObj(std::make_unique<Billboard>(L"Resources/player/player.png", camera))
	, gameCamera(camera)
{
	constexpr float scale = 100.0f;
	gameObj->add(XMFLOAT3(), scale, 0.f);

	loadYamlFile();

	
	// 判定仮設定
	sphere.radius = scale / 2;

	// 追従させるためにポインタを渡す
	//gameCamera->setParentObj(gameObj.get());
}

void Player::update()
{
	// ベクトル計測用
	preFramePos = currentFramePos;
	currentFramePos = XMFLOAT2(mapPos.x, mapPos.y);

	move();
	jump();
	rebound();

	//// 一旦ここに書いてる
	//if(terrainHitObjPosX != 0.f)
	//{
	//	gameObj->setPosition(XMFLOAT3(terrainHitPosX, 0.f, 0.f));
	//}

	
	DirectX::XMFLOAT3 objPos = getObj()->position;
	sphere.center.m128_f32[0] = objPos.x; 
	sphere.center.m128_f32[1] = objPos.y;
	getObj()->position = XMFLOAT3(mapPos.x, mapPos.y, getObj()->position.z);

	gameObj->update(XMConvertToRadians(getObj()->rotation));
}

void Player::draw()
{
	gameObj->draw();
}

void Player::hit()
{
	gameCamera->changeStateGoal();
}

void Player::calcJumpPos()
{
	fallTime++;

	const float PRE_VEL_Y = currentFallVelovity;
	currentFallVelovity = calcFallVelocity(fallStartSpeed, gAcc, fallTime);
	const float ADD_VEL_Y = currentFallVelovity - PRE_VEL_Y;

	//毎フレーム速度を加算
	mapPos.y += currentFallVelovity;
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
		} else// 通常ジャンプ
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

	// 仮に0.f以下になったらジャンプ終了
	if (mapPos.y < 0.f)
	{
		// ジャンプ終了処理
		isJump = false;
		fallTime = 0;
		mapPos.y = 0.f;

		isDrop = false;

		startRebound();
	}
}

void Player::calcDropVec()
{
	preFramePos.y = currentFramePos.y;
	currentFramePos.y = mapPos.y;

	// preの方が大きい(落下開始)し始めたら代入
	if (currentFramePos.y > preFramePos.y && !isDrop)
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
	if (mapPos.x >= testP)
	{

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
	// 仮に0.f以下になったらジャンプ終了
	if (mapPos.y < 0.f)
	{
		fallTime = 0;
		mapPos.y = 0.f;
		constexpr float boundEndVel = 3.f;
		if (-currentFallVelovity <= boundEndVel)
		{
			isReboundY = false;
			isDrop = false;
		} else
		{
			startRebound();
		}
	}
}

void Player::calcSideRebound()
{
	if (!isReboundX)return;

	// 座標に加算
	mapPos.x += sideAddX;

	// 加算値を加算または減算

	// 変化する値
	constexpr float changeValue = 1.0f;
	if (sideAddX > 0)
	{
		sideAddX -= changeValue;

		// 負の値になったら演算終了
		if (sideAddX <= 0)
		{
			sideAddX = 0;
			isReboundX = false;
		}
	} else
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
	// ここに衝突したときの座標格納する
	terrainHitPosX = 80.0f;

	getObj()->position.x = terrainHitPosX;

	// 壁の隣に移動
	const XMFLOAT3 clampPos = getObj()->position;
	currentFramePos = XMFLOAT2(clampPos.x, clampPos.y);

	// 進行方向を求めるためにベクトルを求める
	const float vec = preFramePos.x - currentFramePos.x;

	// 速度に合わせて代入
	constexpr float mag = 1.f;
	sideAddX = vec * mag;

	isReboundX = true;

	terrainHitObjPosX = mapPos.x;
}

void Player::move()
{
	// 角度を取得
	const float angle = gameCamera->getAngle();

	// 角度に応じて移動
	// 一旦加速は考慮せずに実装
	// ここ変更すると速度が変わる
	constexpr float speedMag = 0.35f;

	const float addPos = angle * speedMag;

	DirectX::XMFLOAT3 position = getObj()->position;
	position.x += addPos;


	// 加速度計算と加算
	// 最大速度
	constexpr float maxSpeed = 5.f;
	if (abs(preFramePos.x - position.x) <= maxSpeed)
	{
		constexpr float accMag = 0.015f;
		acc += addPos * accMag;
		position.x += acc;

		// 停止したらリセット
		// ここジャストじゃなくて一定範囲でもいいかも
		if (preFramePos.x == currentFramePos.x)acc = 0.f;
	}


	// 計算後セット
	getObj()->position = position;
	
	// 回転
	rot();
}

void Player::rot()
{
	// 直径
	const float d = getObj()->scale;
	const float ensyuu = d * XM_PI;

	constexpr float angleMax = 360.f;
	const float angleVec = -getObj()->position.x / ensyuu * angleMax;

	// 角度計算
	getObj()->rotation = std::fmod(angleVec, angleMax);
}