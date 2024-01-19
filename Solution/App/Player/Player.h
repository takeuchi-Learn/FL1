/*****************************************************************//**
 * \file   Player.h
 * \brief  自機クラス
 *********************************************************************/

#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include <3D/Billboard/Billboard.h>
#include <Collision/CollisionShape.h>
#include "../GameCamera.h"

class Camera;
class ObjModel;
class Light;
class SoundData;

class Player
{
	class YamlData
	{
	public:
		YamlData() = default;
		~YamlData() = default;

		/// @brief データをYAMLファイルから読み込む
		/// @return エラーがあったかどうか（エラーでtrue）
		bool loadYamlFile();

		// yamlから読む変数
		float jumpPower{};
		float bigJumpPower{};
		float jumpSensorValue{};
		float bigSensorJyroValue{};
		float boundEndVel{};
		float speedMag{};
		float fallVelMag{};
		float sideReboundAddVal{};
		float maxSpeedX{};
		float accMagX{};
		uint8_t tutorialStageMax = 0ui8;
	};

	DirectX::XMFLOAT2 mapPos{};
	std::unique_ptr<Billboard> gameObj;

	GameCamera* camera = nullptr;

	CollisionShape::Sphere sphere;

	std::unique_ptr<YamlData> yamlData;

	// 現在のフレームでジャンプキー押したかどうか(ジャンプできない不具合防止用)
	bool pushJumpKeyFrame = false;
	bool reboundYFrame = false;

	//落下フレーム
	int fallFrame = 0;
	//落下初速
	float fallStartSpeed = 0.0f;
	//現在の落下速度
	float currentFallVelovity = 0.0f;
	// 重力加速度(一旦プレイヤー内に宣言)
	static constexpr float gAcc = 0.35f;

	// 落下中かどうか(跳ね返り用)
	bool isDrop = false;
	//ジャンプしているかどうか
	bool isJump = true;

	// 跳ね返りしているかどうか(上下)
	bool isReboundY = false;
	// 横
	bool isReboundX = false;
	// 落下開始した時の高さ(跳ね返り用)
	float dropStartY = 0.f;
	// 横跳ね返り時に加算する値
	float sideAddX = 0.0f;

	// 加速度
	float acc = 0.f;

	// 前フレームの座標(跳ね返り用)
	DirectX::XMFLOAT2 preFramePos = { 0.f,0.f };
	// 現フレームの座標(跳ね返り用)
	DirectX::XMFLOAT2 currentFramePos = { 0.f,0.f };

	/// @brief 座標がこの数値を下回ったら落下死
	float gameoverPos = 0.f;
	bool isDead = false;

	// スクロール止める左側の座標(mapPosがこれを下回った場合追従をオフにする)
	float leftScrollEndPos = 0.f;
	// 上の右版
	float rightScrollEndPos = 0.f;

	bool setMoveLimitFlag = false;

	// コーンのカウント
	uint16_t coneCount = 0;

	DirectX::XMVECTOR preColliderPos;

#pragma region SE
	std::weak_ptr<SoundData> jumpSE;
	std::weak_ptr<SoundData> boundXSE;
	std::weak_ptr<SoundData> boundYSE;

#pragma endregion

private:
	void loadSE();

	/// @brief ジャンプ中の座標更新処理
	void calcJumpPos();
	// ジャンプ
	void jump();
	// ジャンプ終了確認
	void jumpEnd(const CollisionShape::AABB& hitAABB);

	// 落下ベクトル計測
	void calcDropVec();
	// 跳ね返り
	void rebound();

	// 跳ね返り開始
	void startRebound();
	// 跳ね返り終了確認
	void reboundEnd(const CollisionShape::AABB& hitAABB);
	// 横跳ね返り計算
	void calcSideRebound();
	/// @brief 横跳ね返り開始。これを衝突したときに呼び出す。
	void startSideRebound(float wallPosX, bool hitLeft);

	// 移動
	void move();
	// 回転
	void rot();
	/// @brief ステージの端にいるかの確認(スクロール停止用)
	void checkStageSide();

	/// @brief 移動制限設定
	void moveLimit();
public:
	// 物理挙動をするかどうか
	bool isDynamic = true;

	/// @brief 入力を許可するか
	/// todo 仮実装。操作を別クラスに分けたら消す
	bool allowInput = true;

	/// @brief isReboundXとisReboundYをfalseにする
	inline void resetReboundFlag()
	{
		isReboundX = false;
		isReboundY = false;
	}

	// コンストラクタ仮
	Player(GameCamera* camera);

	// 更新
	void update();
	// 描画
	void draw();

	inline void setScrollendPosRight(float pos) { rightScrollEndPos = pos; }

	void setMapPos(const DirectX::XMFLOAT2& mapPos);

	/// @brief ゲームオーバー扱いになる座標をセットする関数
	/// @param posY
	inline void setGameOverPos(float posY) { gameoverPos = posY; }

	inline const auto& getObj() const { return gameObj->getFrontData(); }

	inline const auto& getMapPos() const { return mapPos; }

	inline bool getIsDead() const { return isDead; }

	inline float getStartPosX() const { return leftScrollEndPos; }

	inline uint16_t getConeCount() const { return coneCount; }

	inline void incrementConeCount() { ++coneCount; }

	inline const auto& getYamlData() const { return yamlData; }

	void hitMap(const CollisionShape::AABB& hitAABB, uint8_t validCollisionDir = 0b1111ui8);

	/// @brief 当たり判定取得
	/// @return 当たり判定の情報
	const auto& getShape() const { return sphere; }
};
