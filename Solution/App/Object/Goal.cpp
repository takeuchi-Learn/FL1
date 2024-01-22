#include "Goal.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>

#include <GameObject/AbstractGameObj.h>
#include <3D/Obj/ObjModel.h>
#include <3D/Light/Light.h>

#include<Player/Player.h>

using namespace DirectX;


// ここのパスマップのデータから読み取ったやつを使ったほうがいいから変更する
// テクスチャをMapじゃない別のフォルダにオブジェクト移したほうがいいかも

void Goal::movePlayer()
{



}

void Goal::departure()
{
	// 移動
	constexpr float frameAddSpeed = 0.25f;
	speed += frameAddSpeed;
	gameObj->getFrontData()->position.x += speed;
	tireObj->getFrontData()->position.x += speed;
	player->getObj()->position.x += speed;

	// 回転 
	const float angleDeg = 1.5f * speed;
	tireObj->getFrontData()->rotation += angleDeg;
	player->getObj()->rotation += angleDeg;
}

Goal::Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale)
	:StageObject(camera, pos, scale * 5.f, L"Resources/Map/Tex/Goal_Car.png")
	,tireObj(std::make_unique<Billboard>(L"Resources/player/player.png", camera))
{
	// 補整座標(ymlで指定できるようにするための補正)
	constexpr float addObjPosY = 242.f;
	gameObj->getFrontData()->position.y += addObjPosY;

	// 判定大きさ決め
	const float harfSize = scale / 2;
	XMFLOAT2 minPos(pos.x - harfSize, pos.y - harfSize);
	XMFLOAT2 maxPos(pos.x + harfSize, pos.y + harfSize);

	// 位置ずらし
	constexpr XMFLOAT2 addAABBPos(-145.f, 0);
	minPos.x += addAABBPos.x;
	minPos.y += addAABBPos.y;
	maxPos.x += addAABBPos.x;
	maxPos.y += addAABBPos.y;

	// 判定セット
	aabb.minPos = XMLoadFloat2(&minPos);
	aabb.maxPos = XMLoadFloat2(&maxPos);

	// 仮　判定削除用
	/* minPos(-999,-999);
	  maxPos(-999, -999);
	aabb.minPos = XMLoadFloat2(&minPos);
	aabb.maxPos = XMLoadFloat2(&maxPos);*/


	// タイヤ追加&座標セット
	XMFLOAT2 tirePos = pos;
	tirePos.x += 175.f;
	tirePos.y += -10.f;
	tireObj->add(XMFLOAT3(tirePos.x, tirePos.y, 0), player->getObj()->scale);


#ifdef _DEBUG
	// ゴールの判定確認用

	aabbObj = std::make_unique<Billboard>(L"Resources/Map/Tex/goal.png", camera);
	
	XMFLOAT3 point
	(
		aabb.maxPos.m128_f32[0] - (aabb.maxPos.m128_f32[0] - aabb.minPos.m128_f32[0]) / 2,
		aabb.maxPos.m128_f32[1] - (aabb.maxPos.m128_f32[1] - aabb.minPos.m128_f32[1]) / 2,
		aabb.maxPos.m128_f32[2] - (aabb.maxPos.m128_f32[2] - aabb.minPos.m128_f32[2]) / 2
	);
	aabbObj->add(point, scale);

#endif // _DEBUG

}

void Goal::update()
{
	gameObj->update(getCameraAngleDeg());
	tireObj->update(getCameraAngleDeg() + tireObj->getFrontData()->rotation);
#ifdef _DEBUG
	aabbObj->update(getCameraAngleDeg());
#endif // _DEBUG

	// 発進処理
	if (isGoal && !isDeparture)
	{
		movePlayer();
		// プレイヤーが穴にハマったら発車
		if(1) isDeparture = true;
	}
	if (isDeparture)departure();

}

void Goal::draw()
{
	// タイヤの描画と本体の描画を行う
	gameObj->draw();
	tireObj->draw();

#ifdef _DEBUG
	aabbObj->draw();
#endif // _DEBUG
}

void Goal::hit(const CollisionShape::Sphere& playerSphere)
{
	isGoal = true;
}
