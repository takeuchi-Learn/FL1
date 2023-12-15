#pragma once

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
	std::vector<CollisionShape::AABB> mapAABBs;
	// 縦横サイズ
	unsigned int mapSizeX = 0;
	unsigned int mapSizeY = 0;

	// ステージの配置物
	std::vector<std::unique_ptr<StageObject>>stageObjects;
	float goalPosX = 0.f;
	

	// コーンの最大値
	unsigned short coneMax = 0;

	GameCamera* camera = nullptr;

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
	const std::vector<CollisionShape::AABB>& getMapAABBs() const { return mapAABBs; }

	/// @brief 仮のゴール当たり判定取得(後々StageObjectを継承して配列にまとめて取得できるようにします)
	//const CollisionShape::AABB& getGoalAABB()const { return goal->getRefAABB(); }
	const std::vector<std::unique_ptr<StageObject>>& getStageObjects()const { return stageObjects; }


	/// @brief ゲームオーバーになる座標
	float getGameoverPos() const;

	float getGoalPosX() const { return goalPosX; }

	unsigned int getMapX() const { return mapSizeX; }
	unsigned int getMapY()const { return mapSizeY; }

	unsigned short getConeMax()const { return coneMax; }

};
