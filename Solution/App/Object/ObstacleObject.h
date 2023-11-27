#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include<GameObject/AbstractGameObj.h>

#include<Collision/CollisionShape.h>

class ObjModel;
class GameCamera;
class Billboard;

// 障害物
class ObstacleObject
{
private:

public:
	ObstacleObject(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	void update();
	void draw();
};

