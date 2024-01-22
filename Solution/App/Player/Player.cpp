#include "Player.h"
#include <Camera/CameraObj.h>
#include <cmath>
#include <Input/Input.h>
#include <Util/Timer.h>
#include <fstream>
#include <Util/YamlLoader.h>
#include <3D/Light/Light.h>
#include <Input/PadImu.h>
#include<JumpVectorCalculation.h>
#include<Sound/SoundData.h>

#include <GameMap.h>
#include <Object/Goal.h>
#include<Object/ColorCone.h>


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

Player::HIT_AREA Player::checkHitArea(const CollisionShape::AABB& hitAABB)
{
	HIT_AREA activeDir = HIT_AREA::NONE;


	// todo XVectorGetX等を使う
	const XMFLOAT3 boxSize(hitAABB.maxPos.m128_f32[0] - hitAABB.minPos.m128_f32[0],
						   hitAABB.maxPos.m128_f32[1] - hitAABB.minPos.m128_f32[1],
						   hitAABB.maxPos.m128_f32[2] - hitAABB.minPos.m128_f32[2]);
	const XMFLOAT3 boxCenter(hitAABB.maxPos.m128_f32[0] - boxSize.x / 2.f,
							 hitAABB.maxPos.m128_f32[1] - boxSize.y / 2.f,
							 hitAABB.maxPos.m128_f32[2] - boxSize.z / 2.f);

	XMFLOAT3 spherePos{};
	XMStoreFloat3(&spherePos, sphere.center);

	const auto boxCenter2Sphere =
		XMFLOAT3(spherePos.x - boxCenter.x,
				 spherePos.y - boxCenter.y,
				 spherePos.z - boxCenter.z);

	// 衝突した辺を四方向でとる
	// 上下と左右どちらかだけにする
	if (std::abs(boxCenter2Sphere.x) >=
		std::abs(boxCenter2Sphere.y))
	{
		// x > yならx方向のみ判定
		if (0.f <= boxCenter2Sphere.x)
		{
			activeDir = HIT_AREA::RIGHT;
		} else if (boxCenter2Sphere.x <= -0.f)
		{
			activeDir = HIT_AREA::LEFT;
		}
	} else
	{
		// y > xならy方向のみ判定
		if (0.f < boxCenter2Sphere.y)
		{
			activeDir = HIT_AREA::TOP;
		} else if (boxCenter2Sphere.y <= 0.f)
		{
			activeDir = HIT_AREA::BOTTOM;
		}
	}

	return activeDir;
}

void Player::hitGround(const CollisionShape::AABB& hitAABB)
{
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
}

void Player::checkGoalDir(const CollisionShape::AABB& goalAABB)
{
	// 自身の中心とゴール判定の中心の距離が近くなったら(誤差1.0くらい)if文の中に入る
	if(1)
	{
		justGoalPoint = true;
		sphere.radius = 0.f;
	}
}

Player::Player(GameCamera* camera) :
	gameObj(std::make_unique<Billboard>(L"Resources/player/player.png", camera))
	, camera(camera)
{
	constexpr float scale = mapSize * 0.9f;
	gameObj->add(XMFLOAT3(), scale, 0.f);

	loadYamlFile();
	loadSE();

	// 判定仮設定
	constexpr float r = scale / 2.f;
	sphere.radius = r;
}

void Player::update()
{
	if (justGoalPoint)
	{
		gameObj->update(XMConvertToRadians(getObj()->rotation));
		++goalMoveTime;
		return;
	}

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
	if (hitObjName == typeid(Goal).name()) // ゴール衝突
	{
		camera->changeStateClear();
		isClear = true;

		// ゴール演出の為の処理
		checkGoalDir(hitAABB);

		// 仮
		justGoalPoint = true;
		// 本来は自動移動終了後に行う(これしないと地面にぶつかった際にposに値が代入されるので、移動の制御が行えない)
		sphere.radius = 0.f;

	} 
	else if (hitObjName == std::to_string(GameMap::MAPCHIP_DATA::MAPCHIP_BLOCK)
			 || hitObjName == std::to_string(GameMap::MAPCHIP_DATA::MAPCHIP_WALL)
			 || hitObjName == std::to_string(GameMap::MAPCHIP_DATA::MAPCHIP_OBSTACLE_OBJECT)
			 || hitObjName == std::to_string(GameMap::MAPCHIP_DATA::MAPCHIP_PLAIN_ROAD)) // マップとの衝突
	{
		// 上のif後で修正します。許して

		// 衝突位置確認
		HIT_AREA activeDir = checkHitArea(hitAABB);

		// 方向ごとに分岐
		switch (activeDir)
		{
			using enum HIT_AREA;
		case TOP:
			// 地面衝突
			hitGround(hitAABB);
			break;

		case BOTTOM:
			// 下方向に落下
			fallStartSpeed = -0.2f;
			fallTime = 0;
			break;

		case RIGHT:
			// 横のバウンド開始
			startSideRebound(XMVectorGetX(hitAABB.maxPos), false);
			break;

		case LEFT:
			// 横のバウンド開始
			startSideRebound(XMVectorGetX(hitAABB.minPos), true);
			break;

		default:
			break;
		}

		if (activeDir != HIT_AREA::NONE)
		{
			getObj()->position = XMFLOAT3(mapPos.x, mapPos.y, getObj()->position.z);
			gameObj->update(XMConvertToRadians(getObj()->rotation));
		}
	}
	else if (hitObjName == typeid(ColorCone).name())
	{
		++coneCount;
	}
	else if(hitObjName == std::to_string(GameMap::MAPCHIP_DATA::MAPCHIP_SLIPTHROUGH_FLOOR))
	{
		// 衝突位置確認
		HIT_AREA activeDir = checkHitArea(hitAABB);
		
		// 上に衝突かつ下に進行してたら(落下していたら)地面衝突処理
		if(activeDir == HIT_AREA::TOP
		   && currentFallVelovity <= 0)
		{
			hitGround(hitAABB);
		}
	}
}

void Player::calcJumpPos()
{
	if (!isDynamic) { return; }

	++fallTime;

	const float PRE_VEL_Y = currentFallVelovity;
	currentFallVelovity = JumpVectorCalculation::calcFallVector(fallStartSpeed, gAcc, fallTime);
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
	if (abs(sideAddX) >= 14.f)
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

	setMoveLimitFlag = true;
}