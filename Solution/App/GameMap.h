#pragma once

#include <memory>
#include <string>
#include <forward_list>
#include <unordered_map>
#include <functional>
#include <DirectXMath.h>
#include <Util/Util.h>
#include <Collision/CollisionShape.h>
#include <Util/YamlLoader.h>

class Goal;
class ColorCone;
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
	std::forward_list<std::unique_ptr<Goal>> goals;
	std::forward_list<std::unique_ptr<ColorCone>> cones;

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

	void loadStageObjList(const std::string& csvStr, float scale, const std::function<void(const DirectX::XMFLOAT2&)> insertFunc);

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

	inline const auto& getGoals() const { return goals; }
	inline auto& getGoals() { return goals; }

	inline auto& getCones() { return cones; }

	/// @brief ゲームオーバーになる座標
	float calcGameoverPos() const;

	inline uint16_t getMapX() const { return mapSizeX; }
	inline uint16_t getMapY() const { return mapSizeY; }

	uint16_t getConeMax() const { return coneMax; }
};
