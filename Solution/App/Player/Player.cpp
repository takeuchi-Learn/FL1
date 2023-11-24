#include "Player.h"
#include <Camera/CameraObj.h>
#include <cmath>
#include <Input/Input.h>
#include <Util/Timer.h>
#include <fstream>
#include <Util/YamlLoader.h>
#include <3D/Light/Light.h>
#include <Input/PadImu.h>

#include <GameMap.h>
#include <Object/Goal.h>
#include<Sound/SoundData.h>


using namespace DirectX;

namespace
{
	// todo mapSizeはGameMapのconstexpr定数にする
	constexpr float mapSize = 100.f;

	constexpr auto jumpSEPath = "Resources/SE/Player/jump.wav";
	constexpr auto boundXSEPath = "Resources/SE/Player/boundX.wav";
	constexpr auto boundYSEPath = "Resources/SE/Player/boundY.wav";

	constexpr float defaultJumpPower = 15.f;
}

bool Player::loadYamlFile()
{
	constexpr const char filePath[] = "Resources/DataFile/player.yml";

	Yaml::Node root{};

	if (YamlLoader::LoadYamlFile(root, filePath))
	{
		return true;
	}

	LoadYamlData(root, jumpPower);
	LoadYamlData(root, bigJumpPower);
	LoadYamlData(root, jumpSensorValue);
	LoadYamlData(root, bigSensorJyroValue);
	LoadYamlData(root, boundEndVel);
	LoadYamlData(root, speedMag);
	LoadYamlData(root, fallVelMag);
	LoadYamlData(root, sideReboundAddVal);
	LoadYamlData(root, maxSpeedX);
	LoadYamlData(root, accMagX);

	return false;
}

void Player::loadSE()
{
	jumpSE = Sound::ins()->loadWave(jumpSEPath);
	boundXSE = Sound::ins()->loadWave(boundXSEPath);
	boundYSE = Sound::ins()->loadWave(boundYSEPath);
}

Player::Player(GameCamera* camera) :
	gameObj(std::make_unique<Billboard>(L"Resources/player/player.png", camera))
	, camera(camera)
{
	constexpr float scale = mapSize;
	gameObj->add(XMFLOAT3(), scale, 0.f);

	loadYamlFile();
	loadSE();

	// 判定仮設定
	constexpr float r = scale / 2.f * 0.8f;
	sphere.radius = r;
}

void Player::update()
{
	// ベクトル計測用
	preFramePos = currentFramePos;
	currentFramePos = XMFLOAT2(mapPos.x, mapPos.y);

	move();
	rot();

	// スタート時はジャンプしないように
	if (camera->getCameraState() != GameCamera::CameraState::START)
	{
		moveLimit();

		jump();
		rebound();
	}

	getObj()->position = XMFLOAT3(mapPos.x, mapPos.y, getObj()->position.z);
	// todo 本来下のようであるべき（mapPosはマップチップでの座標であってワールド座標ではない）
	/*getObj()->position = XMFLOAT3(mapPos.x * mapSize,
								  mapPos.y * mapSize,
								  getObj()->position.z);*/
	sphere.center = XMLoadFloat3(&getObj()->position);

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

void Player::setMapPos(const DirectX::XMFLOAT2& mapPos)
{
	XMFLOAT2 pos = mapPos;

	// todo mapPosの扱いを正したら消す
	constexpr float shiftVal = mapSize;
	pos.x *= shiftVal;
	pos.y *= shiftVal;

	this->mapPos = pos;
	getObj()->position = XMFLOAT3(pos.x, pos.y, getObj()->position.z);
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
			using enum HIT_AREA;
		case UP:
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
		case DOWN:
			// 下方向に落下
			fallStartSpeed = -0.2f;
			fallTime = 0;
			break;
		case LEFT:
			// 横のバウンド開始
			startSideRebound(XMVectorGetX(hitAABB.minPos), true);

			break;
		case RIGTH:
			// 横のバウンド開始
			startSideRebound(XMVectorGetX(hitAABB.maxPos), false);
			break;
		}

		getObj()->position = XMFLOAT3(mapPos.x, mapPos.y, getObj()->position.z);
		gameObj->update(XMConvertToRadians(getObj()->rotation));
	} else if (hitObjName == typeid(Goal).name())// ゴール衝突
	{
		camera->changeStateClear();
		isClear = true;
	}
}

void Player::calcJumpPos()
{
	if (!isDynamic) { return; }

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
	float sensorValue = camera->getGetAccelZ();
	if (PadImu::ins()->getDevCount() > 0)
	{
		const auto state = JslGetIMUState(PadImu::ins()->getHandles()[0]);
		sensorValue = state.accelY / 8.f;
	}

	calcDropVec();

	pushJumpKeyFrame = false;

	if (!isJump && fallTime < 1 || isReboundY)
	{
		bool triggerJump = Input::ins()->triggerKey(DIK_Z);
		triggerJump |= Input::ins()->hitInputPadLT() || Input::ins()->hitInputPadRT();

		if (PadImu::ins()->getDevCount() > 0)
		{
			const auto& state = PadImu::ins()->getStates()[0];

			// 左右どちらかのトリガーが半分以上押されていたらジャンプ
			triggerJump |= state.lTrigger >= 0.5f || state.rTrigger >= 0.5f;
		}

		if (triggerJump || sensorValue >= jumpSensorValue)
		{
			pushJumpKeyFrame = true;
			isJump = true;
			fallTime = 0;

			// 大ジャンプ
			bool hitBigJump = Input::ins()->hitKey(DIK_X);
			hitBigJump |= Input::ins()->triggerPadButton(XINPUT_GAMEPAD_RIGHT_SHOULDER);
			if (hitBigJump || sensorValue >= bigSensorJyroValue)
			{
				fallStartSpeed = bigJumpPower;
			} else// 通常ジャンプ
			{
				// 初速度を設定
				// ジャイロの値を取得できるようになったらここをジャイロの数値を適当に変換して代入する
				fallStartSpeed = jumpPower;
			}

			// バウンド強制終了
			isReboundY = false;

			Sound::playWave(jumpSE, 0, 0.2f);
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

	// 押し出し後の位置
	const float extrusionEndPosY = hitAABB.maxPos.m128_f32[1] + sphere.radius;
	//mapPos.y = mapPos.y + hitPosY;
	mapPos.y = extrusionEndPosY + 0.01f;

	isDrop = false;

	if (fallTime > 2)
	{
		startRebound();
	}
	fallTime = 0;
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

	// 落下した高さに合わせて跳ね返り量を変える
	fallStartSpeed = -currentFallVelovity * fallVelMag;

	reboundYFrame = true;

	// 高さがあったら鳴らす
	if (abs(currentFallVelovity) >= 8.f)
	{
		Sound::playWave(boundYSE, 0, 0.2f);
	}
}

void Player::reboundEnd(const CollisionShape::AABB& hitAABB)
{
	fallTime = 0;

	// 押し出し後の位置
	const float extrusionEndPosY = hitAABB.maxPos.m128_f32[1] + sphere.radius;
	//mapPos.y = mapPos.y + hitPosY;
	mapPos.y = extrusionEndPosY + 0.01f;

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
	if (sideAddX > 0)
	{
		sideAddX -= sideReboundAddVal;

		// 負の値になったら演算終了
		if (sideAddX <= 0)
		{
			sideAddX = 0;
			isReboundX = false;
		}
	} else
	{
		sideAddX += sideReboundAddVal;

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
	} else
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

	// 速度があったら鳴らす
	if(abs(sideAddX) >= 14.f)
	{
		Sound::playWave(boundXSE, 0, 0.2f);
	}
}

void Player::move()
{
	if (!isDynamic) { return; }

	// 角度を取得
	const float angle = camera->getAngle();

	const float addPos = angle * speedMag;

	DirectX::XMFLOAT2 position = mapPos;
	position.x += addPos;

	// 加速度計算と加算
	// 最大速度
	if (abs(preFramePos.x - position.x) <= maxSpeedX)
	{
		acc += addPos * accMagX;
		position.x += acc;

		// 停止したらリセット
		// ここジャストじゃなくて一定範囲でもいいかも
		if (preFramePos.x == currentFramePos.x)acc = 0.f;
	}

	// 計算後セット
	mapPos = position;
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
	if (mapPos.x <= leftScrollEndPos || mapPos.x >= rightScrollEndPos)
	{
		camera->setFollowFlag(false);
	} else
	{
		camera->setFollowFlag(true);
	}
}

void Player::checkGameOver()
{
	if (mapPos.y <= gameoverPos)
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