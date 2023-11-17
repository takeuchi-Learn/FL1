﻿#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include<GameObject/AbstractGameObj.h>

#include<Collision/CollisionShape.h>

class Light;
class ObjModel;
class GameCamera;
class Billboard;

class Goal
{
private:
	// ここBillboardクラスに直す
	// というかGameMapクラスに内蔵してもいいかも
	std::unique_ptr<Billboard> gameObj;
	GameCamera* camera = nullptr;

	bool isGoal = false;


	CollisionShape::AABB aabb;

private:

	// ゴールフラグをtrueにする
	void trueIsGoal() { isGoal = true; }

public:
	Goal(GameCamera* camera,const DirectX::XMFLOAT2& pos,float scale);
	void update();
	void draw();

	// ゴールフラグがtrueかどうか
	bool getIsGoal() const { return isGoal; }

	/// @brief 当たり判定取得
	/// @return 当たり判定の情報
	const CollisionShape::AABB& getShape()const { return aabb; }
};

