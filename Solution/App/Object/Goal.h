#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class GameCamera;
class AbstractGameObj;
class ObjModel;

// ゴールの仕様案
// ゴールの判定に触れたら自動で動いてはまる ゴールに球の判定持たせておいて、
// プレイヤーの判定の中心と車の判定の中心がある程度近くなるまで移動
// ゴール判定の大きさを自由に設定できるようにするためにy座標だけある程度近づいたら止まるようにする
// 中心がゴールの判定に移動するまで自動で移動
// 車発進 カメラは固定 タイヤは回す 車体は少し揺らしてもいいかも　少しずつ加速
// 画面外に出たあたりでシーンをクリアに

// StageObjectクラスにプレイヤーのポインタ持たせてもいいかも
// それでカラーコーンとかもプレイヤーの速度に合わせて吹っ飛ぶ距離変えられたりするし
// 全部に持たせるくらいならstaticでもいいかも

// ゴールクラス(ゴールの判定+車)
class Goal :public StageObject
{
private:
	// タイヤ
	std::unique_ptr<Billboard>tireObj;


#ifdef _DEBUG
	// 当たり判定確認用
	std::unique_ptr<Billboard> aabbObj;
#endif // _DEBUG


	bool isGoal = false;
	float speed = 0.f;

private:
	/// @brief 車が発射する処理
	void departure();
public:
	Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	~Goal() {}

	void update()override;
	void draw()override;
	void hit(const CollisionShape::Sphere& playerSphere)override;

	// ゴールフラグがtrueかどうか
	bool getIsGoal() const { return isGoal; }

};
