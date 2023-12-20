#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class GameCamera;

class Goal :public StageObject
{
private:
	bool isGoal = false;

private:
	/// @brief ゴールフラグをtrueにする
	inline void goal() { isGoal = true; }

public:
	Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	~Goal() {}

	// ゴールフラグがtrueかどうか
	inline bool getIsGoal() const { return isGoal; }
};
