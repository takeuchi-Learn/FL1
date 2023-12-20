#include "ColorCone.h"
#include <Sound/SoundData.h>
#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>
#include <JumpVectorCalculation.h>

using namespace DirectX;

void ColorCone::blownAway()
{
	if (!isBlownAway) { return; }

	// 移動
	// プレイヤーの位置に応じて移動方向を変える
	// スピードで吹っ飛ぶ勢い変えてもいいかも

	// 仮移動量
	constexpr float frameAddPosX = 4.f;

	XMFLOAT3& refPlayerPos = gameObj->getFrontData()->position;
	// X方向
	if (blownAwayLeft)
	{
		refPlayerPos.x -= frameAddPosX;
	} else
	{
		refPlayerPos.x += frameAddPosX;
	}

	// Y方向
	++fallTime;
	currentFallVector = JumpVectorCalculation::calcFallVector(fallStartSpeed, gAcc, fallTime);
	refPlayerPos.y += currentFallVector;

	// 回転
	constexpr float frameRotNum = 0.1f;
	ragAngle += frameRotNum;
}

void ColorCone::checkDead()
{
	// ifでGameOverPosを下回るか確認する
	// 吹っ飛ばしたと同時に一緒に落ちた場合に消える瞬間が見れなくなるため、この方法が望ましい
}

ColorCone::ColorCone(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale)
	: StageObject(camera, pos, scale, L"Resources/Map/Tex/SafetyCone.png")
	, se(Sound::ins()->loadWave("Resources/SE/Sys_Set03-click.wav"))
{
	// 当たり判定調整
	constexpr float judgmentAdjustmentNum = 30.0f;
	aabb.minPos.m128_f32[0] += judgmentAdjustmentNum;
	aabb.maxPos.m128_f32[0] -= judgmentAdjustmentNum;
}

void ColorCone::update()
{
	blownAway();
	checkDead();
	gameObj->update(calcCameraAngleRad() + ragAngle);
}

void ColorCone::hit(const CollisionShape::Sphere& playerSphere)
{
	// 吹っ飛んだらreturn
	if (isBlownAway) { return; }

	Sound::playWave(se, 0ui32, 0.2f);

	// 吹っ飛ぶ処理有効
	isBlownAway = true;

	fallStartSpeed = 10.f;

	// 右からぶつかってきたら左に吹っ飛ばす
	const float playerX = XMVectorGetX(playerSphere.center);
	const float myPosX = XMVectorGetX(aabb.minPos);
	if (playerX > myPosX)
	{
		blownAwayLeft = true;
	}

	// 判定を無くす(多段カウント防止のため)
	XMFLOAT2 zero(0, 0);
	aabb.minPos = XMLoadFloat2(&zero);
	aabb.maxPos = XMLoadFloat2(&zero);
}