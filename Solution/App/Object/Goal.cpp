#include "Goal.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>

#include <GameObject/AbstractGameObj.h>
#include <3D/Obj/ObjModel.h>
#include <3D/Light/Light.h>

using namespace DirectX;


// ここのパスマップのデータから読み取ったやつを使ったほうがいいから変更する
// テクスチャをMapじゃない別のフォルダにオブジェクト移したほうがいいかも

Goal::Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale)
	:StageObject(camera, pos, scale * 5.f, L"Resources/Map/Tex/Goal_Car.png")
{
	// 補整座標(ymlで指定しやすくするため)
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
#ifdef _DEBUG
	aabbObj->update(getCameraAngleDeg());
#endif // _DEBUG


	gameObj->update(getCameraAngleDeg());

	// 発進処理
	if(isGoal)
	{

	}
}

void Goal::draw()
{
	// タイヤの描画と本体の描画を行う
	gameObj->draw();

#ifdef _DEBUG
	aabbObj->draw();
#endif // _DEBUG
}
