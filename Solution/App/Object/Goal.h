#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class Light;
class ObjModel;
class GameCamera;
class Billboard;


class Goal :public StageObject
{
private:

	bool isGoal = false;

private:
	/// @brief ゴールフラグをtrueにする
	void goal() { isGoal = true; }

public:
	Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	~Goal() {}

	// ゴールフラグがtrueかどうか
	bool getIsGoal() const { return isGoal; }

};
