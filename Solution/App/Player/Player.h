/*****************************************************************//**
 * \file   Player.h
 * \brief  自機クラス
 *********************************************************************/

#pragma once

#include <DirectXMath.h>
#include <functional>
#include <memory>
#include <GameObject/BaseGameObjectHavingHp.h>

class Camera;
class ObjModel;
class Light;

class Player
{
	std::unique_ptr<BaseGameObjectHavingHp> gameObj;
	Light* light = nullptr;

private:
	/// @brief データをYAMLファイルから読み込む
	/// @return エラーがあったかどうか（エラーでtrue）
	bool loadYamlFile();

public:

	// コンストラクタ仮
	Player(Camera* camera, ObjModel* model);

	// 更新
	void update();
	// 描画
	void draw();

	// ジャンプ
	void jump();

	// 移動
	void move();

	inline void setLight(Light* light) { this->light = light; }

	inline const auto& getObj() { return gameObj; }
};

