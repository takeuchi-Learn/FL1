/*****************************************************************//**
 * \file   Player.h
 * \brief  自機クラス
 *********************************************************************/

#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include <GameObject/BaseGameObjectHavingHp.h>
#include"../GameCamera.h"

class Camera;
class ObjModel;
class Light;

class Player
{
	std::unique_ptr<BaseGameObjectHavingHp> gameObj;
	Light* light = nullptr;

	GameCamera* gameCamera = nullptr;

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
	bool isJump = false;
	// 跳ね返りしているかどうか
	bool isRebound = false;
	// 落下開始した時の高さ(跳ね返り用)
	float dropStartY = 0.f;

	// 前フレームの座標(跳ね返り用)
	DirectX::XMFLOAT2 preFramePos = { 0.f,0.f };
	// 現フレームの座標(跳ね返り用)
	DirectX::XMFLOAT2 currentFramePos = { 0.f,0.f };


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
	void updateJumpPos();
	// ジャンプ
	void jump();
	// ジャンプ終了確認
	void checkJumpEnd();

	// 落下ベクトル計測
	void calcDropVec();
	// 跳ね返り
	void rebound();
	// 跳ね返り開始
	void startRebound();
	// 跳ね返り終了確認
	void checkreBoundEnd();
	/// @brief 横跳ね返り
	void sideRebound();

	// 移動
	void move();

	// 回転
	void rot();
public:

	// コンストラクタ仮
	Player(GameCamera* camera, ObjModel* model);

	// 更新
	void update();
	// 描画
	void draw();

	inline void setLight(Light* light) { this->light = light; }

	/// @brief センサーの値格納用
	/// @param value センサーの値
	void setSensorValue(const float value) { sensorValue = value; }
	inline const auto& getObj() { return gameObj; }
};

