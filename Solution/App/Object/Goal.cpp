#include "Goal.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>

#include <GameObject/AbstractGameObj.h>
#include <3D/Obj/ObjModel.h>
#include <3D/Light/Light.h>

using namespace DirectX;


// ここのパスマップのデータから読み取ったやつを使ったほうがいいから変更する
Goal::Goal(GameCamera* camera, Light* light, const DirectX::XMFLOAT2& pos)
	:camera(camera)
	,model(std::make_unique<ObjModel>("Resources/goal/","goal"))
	,light(light)
{
	// 判定位置調整用変数
	constexpr XMFLOAT2 addPos = XMFLOAT2();

	// 判定仮設定
	sphere.center = { pos.x + addPos.x, pos.y + addPos.y, 0, 0 };
	sphere.radius = 120.f;
}

void Goal::update()
{
	obj->update();

	// 発進処理
	if(isGoal)
	{

	}
}

void Goal::draw()
{
	obj->draw(light);
}
