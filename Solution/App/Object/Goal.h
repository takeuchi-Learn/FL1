#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class GameCamera;


// ゴールの仕様案
// ゴールの判定にある程度重なる(プレイヤーとゴール判定の中心同士の距離とかで判定するのがいい?)
// 中心がゴールの判定に移動するまで自動で移動
// 車発進

// 実装メモ 2024/01/12
// 現在は旗だが、車に変更する
// 当たり判定が変わる為、StageObjectの継承を中止する
// gameMapで配置する際は個別に変数作る

// ゴールクラス(ゴールの判定+車)
class Goal :public StageObject
{
private:

	bool isGoal = false;

private:
	/// @brief ゴールフラグをtrueにする
	void goal() { isGoal = true; }

public:
	Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	~Goal() {}

	// ゴールフラグがtrueかどうか
	bool getIsGoal() const { return isGoal; }

};
