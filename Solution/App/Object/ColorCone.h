#pragma once
#include <memory>
#include <DirectXMath.h>
#include <Collision/CollisionShape.h>
#include"StageObject.h"

class GameCamera;

// カラーコーンクラス コインの代わりにするかもしれないし、しれないかもしれない
class ColorCone :public StageObject
{
private:
	// 吹っ飛び中かどうか
	bool isBlownAway = true;
	// 回転角度
	float angle = 0.f;
	// 吹っ飛び向き
	bool blownAwayLeft = false;

	// 物理演算系 プレイヤーの物理演算処理修正の余裕ができるまでこいつに持たせる
	//落下時間
	int fallTime = 0;
	//落下初速
	float fallStartSpeed = 0.0f;
	//現在の落下速度
	float currentFallVector = 0.0f;
	// 重力加速度(一旦ここに宣言)
	const float gAcc = 0.35f;
private:

	// 吹っ飛び更新
	void blownAway();

	// 死亡確認
	void checkDead();
public:
	ColorCone(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
	~ColorCone(){}
	void update();
	void hit(const CollisionShape::Sphere& playerSphere);
};

