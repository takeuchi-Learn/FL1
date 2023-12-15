#pragma once

#include<string>
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
	GameCamera* camera = nullptr;

protected:
	std::unique_ptr<Billboard> gameObj;
	// 当たり判定
	CollisionShape::AABB aabb{};

protected:
	float getCameraAngleDeg();

public:
	StageObject(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale, const std::wstring& texPath);
	virtual ~StageObject() {}

	virtual void update();
	virtual void draw();

	/// @brief 衝突時に呼び出す関数
	/// @param playerSphere プレイヤーの判定
	virtual void hit(const CollisionShape::Sphere& playerSphere){}

	const CollisionShape::AABB& getRefAABB()const { return aabb; }

	
};

