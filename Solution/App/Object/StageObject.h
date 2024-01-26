#pragma once

#include <string>
#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>

class Light;
class GameCamera;
class Billboard;

class Player;

// ステージ配置物の親クラス
class StageObject
{
private:
	GameCamera* camera = nullptr;

protected:
	std::unique_ptr<Billboard> gameObj;
	// 当たり判定
	CollisionShape::AABB aabb{};
	// 0~1で画像のどこを中心とするか
	DirectX::XMFLOAT2 center{};

	static Player* player;
protected:
	float calcCameraAngleRad();

public:
	StageObject(GameCamera* camera, const DirectX::XMFLOAT2& pos, const DirectX::XMFLOAT2& scale, const std::wstring& texPath);
	virtual ~StageObject() {}

	virtual void update();
	virtual void draw();

	/// @brief 衝突時に呼び出す関数
	/// @param playerSphere プレイヤーの判定
	virtual void hit(const CollisionShape::Sphere& playerSphere) {}

	inline const auto& getAABB() const { return aabb; }

	static void setPlayer(Player* p) { player = p; }
	inline const auto& getCenter() const { return center; }
	inline void setCenter(const auto& center) { this->center = center; }

	inline auto& getObj() { return gameObj; }
};
