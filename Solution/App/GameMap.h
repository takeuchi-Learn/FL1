﻿#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <Collision/CollisionShape.h>
#include "Object/Goal.h"

class Billboard;
class GameCamera;
class Collision;
class Goal;

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
		/// @brief 障害物
		MAPCHIP_OBSTACLE_OBJECT,
		/// @brief ステージ外進行防止の透明な壁
		MAPCHIP_TRANSPARENT_BLOCK,
		/// @brief ただのブロック
		MAPCHIP_BLOCK,

		/// @brief 最後の要素
		MAPCHIP_ALLNUM
	};

private:
	std::unordered_map<MAPCHIP_DATA, std::unique_ptr<Billboard>> billboard;
	/// @brief 地形のAABB
	std::vector<std::vector<CollisionShape::AABB>> mapAABBs;

	std::unique_ptr<Goal>goal;
	float goalPosX = 0.f;

	GameCamera* camera = nullptr;

private:
	/// @brief 指定要素のAABB情報を変更
	/// @param x 配列の添え字
	/// @param y 配列の添え字
	/// @param pos 座標
	/// @param scale 大きさ
	void setAABBData(size_t x, size_t y, const DirectX::XMFLOAT3& pos, float scale);

	bool checkTypeAndSetObject(MAPCHIP_DATA data, size_t x, size_t y, const DirectX::XMFLOAT2& pos, float scale);

public:
	GameMap(GameCamera* camera);
	~GameMap() = default;

	/// @brief YAMLファイルから読み込み
	/// @param filePath YAMLファイルパス
	/// @param startPosBuf 開始時のマップ座標を格納する変数
	/// @return エラーがあるかどうか
	bool loadDataFile(const std::string& filePath, DirectX::XMFLOAT2* startPosBuf = nullptr);

	void update();
	void draw();

	/// @brief 当たり判定の取得
	/// @return 当たり判定配列の参照
	const std::vector<std::vector<CollisionShape::AABB>>& getAABBs() const { return mapAABBs; }

	/// @brief 仮のゴール当たり判定取得(後々StageObjectを継承して配列にまとめて取得できるようにします)
	const CollisionShape::AABB& getGoalAABB()const { return goal->getRefAABB(); }

	/// @brief ゲームオーバーになる座標
	float getGameoverPos() const;

	float getGoalPosX() const { return goalPosX; }

	template<size_t ind = 0u>
	size_t getMapX() const { return mapAABBs[ind].size(); }

	size_t getMapY()const { return mapAABBs.size(); }
};
