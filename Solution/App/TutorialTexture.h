#pragma once
#include<memory>

class Billboard;
class GameCamera;

// 仕様
// 個数少ないので、プレイヤーと同じ配置方法を想定して作成
// プレイヤーと同じ方法で作成するが、それだと画像を正方形にしないといけない
// 画像が長方形の場合は、空白増やして正方形にすること

// アニメーションさせるために複数画像用意するの大変だと思うので、プログラムで回転させたり上下に動かしたほうがよさそう



// チュートリアル画像
class TutorialTexture
{
private:
	class State
	{
	protected:
		Billboard* gameObj = nullptr;
	public:
		State(Billboard* gameObj) :gameObj(gameObj) {}
		virtual void update() {}
	};

	class Move :public State
	{
	public:
		Move(Billboard* gameObj):State(gameObj){}
		void update()override;
	};

	class Jump :public State
	{
	public:
		Jump(Billboard* gameObj) :State(gameObj) {}
		void update()override;
	};

	std::unique_ptr<State> state;
	const unsigned short STAGE_NUM;
private:
	std::unique_ptr<Billboard> gameObj;
	GameCamera* camera = nullptr;

private:
	void createState();

public:
	TutorialTexture(GameCamera* camera,unsigned short stageNum);
	~TutorialTexture() {}

	void update();
	void draw();
};

