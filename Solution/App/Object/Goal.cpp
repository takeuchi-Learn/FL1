#include "Goal.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>

#include <GameObject/AbstractGameObj.h>
#include <3D/Obj/ObjModel.h>
#include <3D/Light/Light.h>

using namespace DirectX;


// ここのパスマップのデータから読み取ったやつを使ったほうがいいから変更する
Goal::Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos)
	:StageObject(camera, pos, 800.f, L"Resources/Map/Tex/SafetyCone.png")
{
}

void Goal::update()
{
	gameObj->update(getCameraAngleDeg());

	// 発進処理
	if(isGoal)
	{

	}
}

void Goal::draw()
{
	// タイヤの描画と本体の描画を行う
}
