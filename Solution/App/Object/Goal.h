#pragma once

#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class GameCamera;
class AbstractGameObj;
class ObjModel;
class Light;


// ゴールの仕様案
// ゴールの判定に触れたら自動で動いてはまる ゴールに球の判定持たせておいて、
// プレイヤーの判定の中心と車の判定の中心がある程度近くなるまで移動
// ゴール判定の大きさを自由に設定できるようにするためにy座標だけある程度近づいたら止まるようにする
// 中心がゴールの判定に移動するまで自動で移動
// 車発進 カメラは固定 タイヤは回す 車体は少し揺らしてもいいかも　少しずつ加速
// 画面外に出たあたりでシーンをクリアに

// 実装メモ 2024/01/12
// 上記だと無駄にBillboardを持たせることになってしまうが、実装速度とバグ増加を考慮して一旦継承する
// 画像を無理やり正方形にしてBillboardでもいいようにする

// もしかしたら浮いてしまうので補正する

// ゴールクラス(ゴールの判定+車)
class Goal :public StageObject
{
private:
#ifdef _DEBUG
	// 当たり判定確認用
	std::unique_ptr<Billboard> aabbObj;
#endif // _DEBUG


	bool isGoal = false;
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
