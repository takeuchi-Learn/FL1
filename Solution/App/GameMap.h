#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include<Collision/CollisionShape.h>

class Billboard;
class GameCamera;
class Collision;

// マップクラス
// 動かない地形のみ扱う
class GameMap
{
public:
	static constexpr float chipSize = 100.f;

private:
	std::unordered_map<std::string, std::unique_ptr<Billboard>> billboard;

	/// @brief 地形のAABB
	std::vector<std::vector<CollisionShape::AABB>>mapAABBs;

	GameCamera* camera = nullptr;

private:
	/// @brief AABBのデータをセット
	/// @param x 配列の添え字
	/// @param y 配列の添え字
	/// @param pos 座標
	/// @param scale サイズ
	void setAABBData(size_t x, size_t y, const DirectX::XMFLOAT3& pos, float scale);

public:
	GameMap(GameCamera* camera);
	~GameMap() = default;

	/// @brief YAMLファイルから読み込み
	/// @param filePath YAMLファイルパス
	/// @return エラーがあるかどうか
	bool loadDataFile(const std::string& filePath);

	void update();
	void draw();

	/// @brief 当たり判定の取得
	/// @return 当たり判定配列の参照
	const std::vector<std::vector<CollisionShape::AABB>>& getAABBs()const { return mapAABBs; }
};
