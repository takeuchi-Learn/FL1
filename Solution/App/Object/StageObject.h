#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>

class Light;
class GameCamera;
class Billboard;

// ステージ配置物の親クラス
class StageObject
{
private:
	std::unique_ptr<Billboard> gameObj;
	GameCamera* camera = nullptr;

protected:
	// 当たり判定
	CollisionShape::AABB aabb{};

public:
	StageObject(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	virtual ~StageObject() {}

	virtual void update();
	virtual void draw();

	const CollisionShape::AABB& getRefAABB()const { return aabb; }

};

