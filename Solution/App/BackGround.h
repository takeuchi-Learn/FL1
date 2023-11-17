#pragma once
#include<memory>

class AbstractGameObj;
class ObjModel;
class GameCamera;
class Light;

// 背景クラス
class BackGround
{
public:

private:
	GameCamera* camera = nullptr;
	
	// 背景
	std::unique_ptr<AbstractGameObj> obj;
	std::unique_ptr<ObjModel> model;

	// ライト(使う必要ないがセットが必須なため)
	std::unique_ptr<Light>light;

public:

	/// @brief 読み込み
	BackGround(GameCamera* camera, unsigned int stageNum = 0);

	void update();
	void draw();
};

