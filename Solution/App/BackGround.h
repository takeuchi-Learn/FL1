#pragma once
#include<memory>

class AbstractGameObj;
class ObjModel;
class GameCamera;

// 背景クラス
class BackGround
{
public:

private:
	GameCamera* camera = nullptr;
	
	// 背景
	std::unique_ptr<AbstractGameObj> backGround;
	std::unique_ptr<ObjModel> model;


public:

	/// @brief 読み込み
	BackGround(GameCamera* camera, unsigned int stageNum = 0);

	void update();
	void draw();
};

