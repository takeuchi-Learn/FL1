#pragma once
class GameScene
{
public:
	virtual ~GameScene() {}

	/// @brief 前シーンの終了処理よりも必ず後に実行される
	virtual void start() {}
	virtual void update() = 0; // 実装必須
	virtual void drawObj3d() {}
	virtual void drawFrontSprite() {}
};
