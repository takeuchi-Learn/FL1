﻿#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include<DirectXMath.h>

#include<Util/Util.h>
#include <Collision/CollisionShape.h>
#include <Util/YamlLoader.h>
#include "Object/Goal.h"

class Billboard;
class GameCamera;
class Collision;

using namespace DirectX;

// マップクラス
// 動かない地形のみ扱う
class GameMap
{
public:
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
		/// @brief すり抜け床
		MAPCHIP_SLIPTHROUGH_FLOOR ,
		/// @brief 最後の要素
		MAPCHIP_ALLNUM
	};

private:
	std::unordered_map<MAPCHIP_DATA, std::unique_ptr<Billboard>> billboard;
	/// @brief 地形のAABB
	std::vector<CollisionShape::AABB> mapAABBs;
	/// @brief マップのマップチップデータ
	std::vector<MAPCHIP_DATA>mapMapChipDatas;

	// すり抜け床メモ
	// 地形判定の配列とは別の配列を用意し、すり抜け床だけ隔離する
	// で、GameMapから取得できるようにし、typeid(GameMap)で地形か確認していた部分を地形とすり抜け床で分ける
	// ゴールもこれでいいかもしれない
	
	
	// 縦横サイズ
	unsigned int mapSizeX = 0;
	unsigned int mapSizeY = 0;

	// ゴール
	std::unique_ptr<Goal>goal;
	// ステージの配置物
	std::vector<std::unique_ptr<StageObject>>stageObjects;
	float goalPosX = 0.f;
	

	// コーンの最大値
	unsigned short coneMax = 0;

	GameCamera* camera = nullptr;
	// ゴールなどの配置物用ライト
	Light* light = nullptr;
private:
	/// @brief 指定要素のAABB情報を変更
	/// @param x 配列の添え字
	/// @param y 配列の添え字
	/// @param pos 座標
	/// @param scale 大きさ
	void setAABBData(size_t x, size_t y, const DirectX::XMFLOAT3& pos, float scale);

	void loadStageObject(Yaml::Node& node,float scale);
	void loadStageObjectPosition(const Util::CSVType& posCSV, std::vector<XMFLOAT2>& output);
	void setStageObjects(const std::unordered_map<std::string, std::vector<XMFLOAT2>>& stageObjectPos, float scale);
public:
	/// @brief 
	/// @param camera カメラ
	/// @param light ゴールなどの配置物用ライト
	GameMap(GameCamera* camera,Light* light);
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
	const std::vector<CollisionShape::AABB>& getMapAABBs() const { return mapAABBs; }
	
	const void getMapAABBs(std::vector<CollisionShape::AABB>& aabbs, std::vector<MAPCHIP_DATA>& datas)
	{
		aabbs = mapAABBs;
		datas = mapMapChipDatas;
	}

	const std::unique_ptr<Goal>& getGoal()const { return goal; }
	const std::vector<std::unique_ptr<StageObject>>& getStageObjects()const { return stageObjects; }


	/// @brief ゲームオーバーになる座標
	float getGameoverPos() const;

	float getGoalPosX() const { return goalPosX; }

	unsigned int getMapX() const { return mapSizeX; }
	unsigned int getMapY()const { return mapSizeY; }

	unsigned short getConeMax()const { return coneMax; }

};
