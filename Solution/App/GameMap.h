﻿#pragma once

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
private:
	enum MAPCHIP_DATA : uint8_t
	{
		/// @brief 未定義
		MAPCHIP_UNDEF = 0U,
		/// @brief 壁
		MAPCHIP_WALL,
		/// @brief 道
		MAPCHIP_ROAD,
		/// @brief ゴール
		MAPCHIP_GOAL,
		/// @brief 道路
		MAPCHIP_PLAIN_ROAD,

		/// @brief 最後の要素
		MAPCHIP_ALLNUM
	};

private:
	std::unordered_map<MAPCHIP_DATA, std::unique_ptr<Billboard>> billboard;
	/// @brief 地形のAABB
	std::vector<std::vector<CollisionShape::AABB>>mapAABBs;

	GameCamera* camera = nullptr;

private:
	/// @brief AABBのデータをセット
	/// @param x 配列の添え字
	/// @param y 配列の添え字
	/// @param pos 座標
	/// @param scale サイズ
	void setAABBData(size_t x,size_t y,const DirectX::XMFLOAT3& pos,float scale);


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
