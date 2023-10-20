﻿/*****************************************************************//**
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
	//ジャンプしているかどうか
	bool isJump = false;

private:
	/// @brief データをYAMLファイルから読み込む
	/// @return エラーがあったかどうか（エラーでtrue）
	bool loadYamlFile();

	/// @brief 自由落下の速度を計算します。
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

	// ジャンプ
	void jump();
	// ジャンプ終了確認
	void checkJumpEnd();

	// 移動
	void move();
public:

	// コンストラクタ仮
	Player(GameCamera* camera, ObjModel* model);

	// 更新
	void update();
	// 描画
	void draw();



	inline void setLight(Light* light) { this->light = light; }
};

