#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include "StageObject.h"

class GameCamera;

// ゴールの仕様案
// ゴールの判定に触れたら自動ではまる ゴールに球の判定持たせておいて、
// プレイヤーの判定の中心と車の判定の中心がある程度近くなるまで移動
// ゴール判定の大きさを自由に設定できるようにするためにy座標だけある程度近づいたら止まるようにする
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
	inline void goal() { isGoal = true; }

public:
	Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, const DirectX::XMFLOAT2& scale);
	virtual ~Goal() {}

	// ゴールフラグがtrueかどうか
	inline bool getIsGoal() const { return isGoal; }
};
