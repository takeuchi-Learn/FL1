/*****************************************************************//**
 * \file   Player.h
 * \brief  自機クラス
 *********************************************************************/

#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include <3D/Billboard/Billboard.h>
#include"../GameCamera.h"

#include<Collision/CollisionShape.h>

class Camera;
class ObjModel;
class Light;


class Player
{
	DirectX::XMFLOAT2 mapPos{};
	std::unique_ptr<Billboard> gameObj;

	GameCamera* camera = nullptr;

	CollisionShape::Sphere sphere;

	// 現在のフレームでジャンプキー押したかどうか(ジャンプできない不具合防止用)
	bool pushJumpKeyFrame = false;
	bool reboundYFrame = false;

	//落下時間
	int fallTime = 0;
	//落下初速
	float fallStartSpeed = 0.0f;
	//現在の落下速度
	float currentFallVelovity = 0.0f;
	// 重力加速度(一旦プレイヤー内に宣言)
	const float gAcc = 0.35f;

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
	// 地形衝突時の座標
	float terrainHitObjPosX = 0.f;
	// 衝突した地形の座標
	//float terrainHitPosX = 0.f;

	// 加速度
	float acc = 0.f;


	// 前フレームの座標(跳ね返り用)
	DirectX::XMFLOAT2 preFramePos = { 0.f,0.f };
	// 現フレームの座標(跳ね返り用)
	DirectX::XMFLOAT2 currentFramePos = { 0.f,0.f };

	/// @brief 座標がこの数値を下回ったら落下死
	float gameoverPos = 0.f;
	bool isDead = false;
	bool isClear = false;

	// スクロール止める左側の座標(mapPosがこれを下回った場合追従をオフにする)
	float leftScrollEndPos = 350.f;

	// センサーの値
	float sensorValue = 0.0f;
private:
	/// @brief データをYAMLファイルから読み込む
	/// @return エラーがあったかどうか（エラーでtrue）
	bool loadYamlFile();

	/// @brief 自由落下(投げ上げ)の速度を計算します。
	/// @param startVel 初速度
	/// @param gravAcc 加速度
	/// @param t 時間
	/// @return 
	float calcFallVelocity
	(
		const float startVel,
		const float gravAcc,
		const int t
	)
	{
		return startVel + -gravAcc * static_cast<float>(t);
	}


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
	void startSideRebound(float wallPosX , bool hitLeft);

	// 移動
	void move();
	// 回転
	void rot();
	/// @brief ステージの端にいるかの確認(スクロール停止用)
	void checkStageSide();

	/// @brief ゲームオーバーの確認
	void checkGameOver();


public:

	// コンストラクタ仮
	Player(GameCamera* camera);

	// 更新
	void update();
	// 描画
	void draw();

	/// @brief ゲームオーバー扱いになる座標をセットする関数
	/// @param posY 
	void setGameOverPos(const float posY) { gameoverPos = posY; }

	/// @brief センサーの値格納用
	/// @param value センサーの値
	void setSensorValue(const float value) { sensorValue = value; }
	inline const auto& getObj() { return gameObj->getFrontData(); }

	inline const auto& getMapPos() const { return mapPos; }

	bool getIsDead() const{ return isDead; }
	bool getIsClear()const { return isClear; }

	float getStartPosX()const { return leftScrollEndPos; }
	
	/// @brief 衝突時に呼び出す関数
	/// @param hitAABB 判定
	/// @param hitObjName 相手のクラス名(typeid.name()で取得する)や識別名("map"など)
	void hit(const CollisionShape::AABB& hitAABB,const std::string& hitObjName = "");

	/// @brief 当たり判定取得
	/// @return 当たり判定の情報
	const CollisionShape::Sphere& getShape()const { return sphere; }

};
