#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <DirectXMath.h>
#include <Util/Util.h>
#include <Collision/CollisionShape.h>
#include <Util/YamlLoader.h>
#include "Object/Goal.h"

class Billboard;
class GameCamera;
class Collision;

// マップクラス
// 動かない地形のみ扱う
class GameMap
{
public:
	class Collider
	{
	public:
		CollisionShape::AABB aabb;
		uint8_t collisionDirectionBitFlag;

		Collider(const DirectX::XMFLOAT2& minPos,
				 const DirectX::XMFLOAT2& maxPos,
				 uint8_t collisionBitFlag);

		Collider(const DirectX::XMFLOAT2& minPos,
				 const DirectX::XMFLOAT2& maxPos)
			: Collider(minPos, maxPos, 0b1111ui8)
		{}
	};

private:
	std::unordered_map<std::string, uint8_t> collisionDataList;

	std::unordered_map<uint8_t, std::unique_ptr<Billboard>> billboard;
	/// @brief 地形のAABB
	std::vector<Collider> mapAABBs;

	// 縦横サイズ
	uint16_t mapSizeX = 0;
	uint16_t mapSizeY = 0;

	// ステージの配置物
	std::vector<std::unique_ptr<StageObject>> stageObjects;
	float goalPosX = 0.f;

	// コーンの最大値
	uint16_t coneMax = 0;

	GameCamera* camera = nullptr;

private:
	/// @brief 指定要素のAABB情報を変更
	/// @param x 配列の添え字
	/// @param y 配列の添え字
	/// @param pos 座標
	/// @param scale 大きさ
	void setAABBData(size_t x, size_t y, const DirectX::XMFLOAT3& pos, float scale, uint8_t collisionBitFlag = 0b1111ui8);

	void loadStageObject(Yaml::Node& node, float scale);
	void loadStageObjectPosition(const Util::CSVType& posCSV, std::vector<DirectX::XMFLOAT2>& output);
	void setStageObjects(const std::unordered_map<std::string, std::vector<DirectX::XMFLOAT2>>& stageObjectPos, float scale);

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
	inline const auto& getMapAABBs() const { return mapAABBs; }

	/// @brief 仮のゴール当たり判定取得(後々StageObjectを継承して配列にまとめて取得できるようにします)
	//inlnie const auto& getGoalAABB() const { return goal->getRefAABB(); }

	inline const auto& getStageObjects() const { return stageObjects; }

	/// @brief ゲームオーバーになる座標
	float calcGameoverPos() const;

	inline float getGoalPosX() const { return goalPosX; }

	inline uint16_t getMapX() const { return mapSizeX; }
	inline uint16_t getMapY() const { return mapSizeY; }

	uint16_t getConeMax() const { return coneMax; }
};
