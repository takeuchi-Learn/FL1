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

private:
	/// @brief データをYAMLファイルから読み込む
	/// @return エラーがあったかどうか（エラーでtrue）
	bool loadYamlFile();

public:

	// コンストラクタ仮
	Player(GameCamera* camera, ObjModel* model);

	// 更新
	void update();
	// 描画
	void draw();

	// ジャンプ
	void jump();

	// 移動
	void move();

	inline void setLight(Light* light) { this->light = light; }
};

