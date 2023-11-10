#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include<GameObject/AbstractGameObj.h>

#include<Collision/CollisionShape.h>

class Light;
class ObjModel;
class Camera;

class Goal
{
private:
	std::unique_ptr<AbstractGameObj> gameObj;
	Light* light = nullptr;

	bool isGoal = false;


	CollisionShape::AABB aabb;

private:

	// ゴールフラグをtrueにする
	void trueIsGoal() { isGoal = true; }

public:
	Goal(ObjModel* model, Camera* camera, Light* light,const DirectX::XMFLOAT2& pos);
	

	// ゴールフラグがtrueかどうか
	bool getIsGoal() const { return isGoal; }
};

