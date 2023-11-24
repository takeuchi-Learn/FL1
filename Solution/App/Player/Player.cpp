#include "Player.h"
#include <Camera/CameraObj.h>
#include <cmath>
#include <Input/Input.h>
#include <Util/Timer.h>
#include <fstream>
#include <Util/YamlLoader.h>
#include <3D/Light/Light.h>

#include<GameMap.h>
#include<Object/Goal.h>

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
	, camera(camera)
{

	constexpr float scale = 100.0f;
	gameObj->add(XMFLOAT3(), scale, 0.f);
	
	loadYamlFile();

	
	// 判定仮設定
	sphere.radius = scale / 2;

}

void Player::update()
{

	// ベクトル計測用
	preFramePos = currentFramePos;
	currentFramePos = XMFLOAT2(mapPos.x, mapPos.y);

	move();

	// スタート時はジャンプしないように
	if(camera->getCameraState() != GameCamera::CameraState::START)
	{
		moveLimit();

		jump();
		rebound();
	}


	sphere.center.m128_f32[0] = mapPos.x;
	sphere.center.m128_f32[1] = mapPos.y;
	getObj()->position = XMFLOAT3(mapPos.x, mapPos.y, getObj()->position.z);


	gameObj->update(XMConvertToRadians(getObj()->rotation));

	// 左右スクロールのオンオフ
	checkStageSide();
	// ゲームオーバー確認
	checkGameOver();

}

void Player::draw()
{
	gameObj->draw();
}

void Player::hit(const CollisionShape::AABB& hitAABB, const std::string& hitObjName)
{
	if (hitObjName == typeid(GameMap).name())
	{
#pragma region 衝突位置確認

		enum class HIT_AREA
		{
			NONE,
			UP,
			DOWN,
			LEFT,
			RIGTH,
		};

		HIT_AREA hitArea = HIT_AREA::NONE;

		//topが1 Xが多い
		//topが2 Yが多い
		short top = 0;


		XMFLOAT3 boxSize
		(
			(hitAABB.maxPos.m128_f32[0] - hitAABB.minPos.m128_f32[0]),
			(hitAABB.maxPos.m128_f32[1] - hitAABB.minPos.m128_f32[1]),
			(hitAABB.maxPos.m128_f32[2] - hitAABB.minPos.m128_f32[2])
		);
		XMFLOAT3 boxPointPos
		(
			hitAABB.maxPos.m128_f32[0] - boxSize.x / 2,
			hitAABB.maxPos.m128_f32[1] - boxSize.y / 2,
			hitAABB.maxPos.m128_f32[2] - boxSize.z / 2
		);


		XMFLOAT3 spherePos
		(
			sphere.center.m128_f32[0],
			sphere.center.m128_f32[1],
			sphere.center.m128_f32[2]
		);
		XMFLOAT3 sphereToVector
		(
			boxPointPos.x - spherePos.x,
			boxPointPos.y - spherePos.y,
			boxPointPos.z - spherePos.z
		);

		if (abs(sphereToVector.x) > abs(sphereToVector.y) &&
			abs(sphereToVector.x) > boxSize.x / 2)
		{
			top = 1;
			if (abs(sphereToVector.z) > abs(sphereToVector.x) &&
				abs(sphereToVector.z) > boxSize.z / 2)
				top = 3;
		} else
		{
			top = 2;
			if (abs(sphereToVector.z) > abs(sphereToVector.y) &&
				abs(sphereToVector.z) > boxSize.z / 2)
				top = 3;
		}

		if (top == 1)
		{
			if (sphereToVector.x >= 0)
			{
				hitArea = HIT_AREA::LEFT;
			} else
			{
				hitArea = HIT_AREA::RIGTH;
			}
		} else if (top == 2)
		{
			if (sphereToVector.y >= 0)
			{
				hitArea = HIT_AREA::DOWN;
			} else
			{
				hitArea = HIT_AREA::UP;
			}
		}

#pragma endregion

		switch (hitArea)
		{
		case HIT_AREA::UP:

			// 地面衝突
			if (!pushJumpKeyFrame || !reboundYFrame)
			{
				// 跳ね返りかジャンプか確認し、それぞれに応じた関数を呼び出す
				if (isReboundY)
				{
					reboundEnd(hitAABB);
				} else
				{
					jumpEnd(hitAABB);
				}
			}
			break;
		case HIT_AREA::DOWN:
			// 下方向に落下
			fallStartSpeed = -0.2f;
			fallTime = 0;
			break;
		case HIT_AREA::LEFT:
			// 横のバウンド開始
			startSideRebound(hitAABB.minPos.m128_f32[0], true);

			break;
		case HIT_AREA::RIGTH:
			// 横のバウンド開始
			startSideRebound(hitAABB.maxPos.m128_f32[0], false);
			break;
		}



		getObj()->position = XMFLOAT3(mapPos.x, mapPos.y, getObj()->position.z);
		gameObj->update(XMConvertToRadians(getObj()->rotation));
	}
    else if (hitObjName == typeid(Goal).name())// ゴール衝突
	{
		camera->changeStateClear();
		isClear = true;
	}
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
	// センサーの値
	const float sensorValue = camera->getGetAccelZ();


	// ジャンプパワー
	constexpr float jumpPower = 10.f;
	constexpr float bigJumpPower = 18.f;


	calcDropVec();

	// ジャンプするのに必要なジャイロの値
	constexpr float jumpSensorValue = 0.25f;
	constexpr float bigSensorJyroValue = 2.f;

	pushJumpKeyFrame = false;


	if (!isJump && fallTime < 1)
	{
		if (Input::getInstance()->triggerKey(DIK_Z) || sensorValue >= jumpSensorValue)
		{
			pushJumpKeyFrame = true;
			isJump = true;
			fallTime = 0;

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

			// バウンド強制終了
			if (isReboundY)
			{
				isReboundY = false;
			}

		}
	}

	if (isReboundY) 
	{
		return;
	}

	if (!isJump)
	{
		// 地形無いときに勝手に落ちるようにするために0.fをセット
		fallStartSpeed = 0.f;
	}

	// ジャンプの更新処理
	calcJumpPos();

	

}

void Player::jumpEnd(const CollisionShape::AABB& hitAABB)
{
	// ジャンプ終了処理
	isJump = false;
	fallTime = 0;

	// 押し出し後の位置
	const float extrusionEndPosY = hitAABB.maxPos.m128_f32[1] + sphere.radius;
	//mapPos.y = mapPos.y + hitPosY;
	mapPos.y = extrusionEndPosY + 0.01f;
	
	isDrop = false;
	startRebound();
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
	reboundYFrame = false;

	// 横バウンド時の座標計算
	// 一旦壁と隣接してるフレームを描画するために先に呼び出している
	calcSideRebound();

	if (!isReboundY)return;
	// 更新
	calcJumpPos();
}

void Player::startRebound()
{
	// 跳ね返り開始処理
	isReboundY = true;

	constexpr float fallVelMag = 0.4f;
	// 落下した高さに合わせて跳ね返り量を変える
	fallStartSpeed = -currentFallVelovity * fallVelMag;

	reboundYFrame = true;
}

void Player::reboundEnd(const CollisionShape::AABB& hitAABB)
{

	fallTime = 0;


	// 押し出し後の位置
	const float extrusionEndPosY = hitAABB.maxPos.m128_f32[1] + sphere.radius;
	//mapPos.y = mapPos.y + hitPosY;
	mapPos.y = extrusionEndPosY + 0.01f;

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

void Player::calcSideRebound()
{
	if (!isReboundX)return;

	// 座標に加算
	mapPos.x += sideAddX;

	// 加算値を加算または減算

	// 変化する値
	constexpr float changeValue = 2.5f;
	if (sideAddX > 0)
	{
		sideAddX -= changeValue;

		// 負の値になったら演算終了
		if (sideAddX <= 0)
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

void Player::startSideRebound(const float wallPosX, bool hitLeft)
{

	// 当たった向きに応じて押し出す
	if (hitLeft)
	{
		mapPos.x = wallPosX - sphere.radius;

	}
	else 
	{
		mapPos.x = wallPosX + sphere.radius;
	}

	// 壁の隣に移動
	const XMFLOAT2 clampPos = mapPos;
	currentFramePos = clampPos;

	// 進行方向を求めるためにベクトルを求める
	const float vec = preFramePos.x - currentFramePos.x;

	// 速度に合わせて代入
	constexpr float mag = 1.f;
	sideAddX = vec * mag;

	isReboundX = true;
	// 衝突時の座標を代入
	terrainHitObjPosX = mapPos.x;
}

void Player::move()
{
	// 角度を取得
	const float angle = camera->getAngle();

	// 角度に応じて移動
	// 一旦加速は考慮せずに実装
	// ここ変更すると速度が変わる
	constexpr float speedMag = 0.35f;

	const float addPos = angle * speedMag;

	DirectX::XMFLOAT2 position = mapPos;
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
	mapPos = position;
	
	// 回転
	rot();
}

void Player::rot()
{
	// 直径
	const float d = getObj()->scale;
	const float ensyuu = d * XM_PI;

	constexpr float angleMax = 360.f;
	const float angleVec = -mapPos.x / ensyuu * angleMax;

	// 角度計算
	getObj()->rotation = std::fmod(angleVec, angleMax);
}

void Player::checkStageSide()
{
	if (isDead)return;


	// 初期位置より下になったら、または、ゴールに近づいたらスクロール停止
	if(mapPos.x <= leftScrollEndPos || mapPos.x >= rightScrollEndPos)
	{
		camera->setFollowFlag(false);
	}
	else
	{
		camera->setFollowFlag(true);
	}

}

void Player::checkGameOver()
{
	if(mapPos.y <= gameoverPos)
	{
		isDead = true;
	}
}

void Player::moveLimit()
{
	if (setMoveLimitFlag)return;

	// 操作可能になったタイミングの座標をセット
	leftScrollEndPos = mapPos.x;
	// ゴールが中心に表示されたらスクロール停止
	rightScrollEndPos = goalPosX;

	setMoveLimitFlag = true;
}

