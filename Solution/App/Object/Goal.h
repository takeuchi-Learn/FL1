#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class GameCamera;


// ゴールの仕様案
// ゴールの判定にある程度重なる(プレイヤーとゴール判定の中心同士の距離とかで判定するのがいい?)
// 中心がゴールの判定に移動するまで自動で移動
// 車発進 カメラは固定 タイヤは回す 車体は少し揺らしてもいいかも　少しずつ加速
// 画面外に出たあたりでシーンをクリアに

// 実装メモ 2024/01/12
// 現在は旗だが、車に変更する
// 当たり判定がAABBから球に変わる為、StageObjectの継承を中止する
// gameMapで配置する際は個別に変数作る
// スプライトが横長なので、3Dの板ポリ(背景と)で実装すること(サイズは800*300)

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
