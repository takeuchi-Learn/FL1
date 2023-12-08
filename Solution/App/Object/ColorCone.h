#pragma once
#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class GameCamera;

// カラーコーンクラス コインの代わりにするかもしれないし、しれないかもしれない
class ColorCone :public StageObject
{
private:
	// 吹っ飛び中かどうか
	bool isBlownAway = true;

private:

	// 吹っ飛び更新
	void blownAway();
public:
	ColorCone(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	~ColorCone(){}
	void update();
	void hit(const CollisionShape::Sphere& playerSphere);
};

