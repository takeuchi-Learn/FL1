#include "Goal.h"

#include <GameCamera.h>
#include <3D/Billboard/Billboard.h>
using namespace DirectX;


// ここのパスマップのデータから読み取ったやつを使ったほうがいいから変更する
Goal::Goal(GameCamera* camera, const DirectX::XMFLOAT2& pos, const float scale)
	:StageObject(camera, pos, scale,L"Resources/Map/Tex/goal.png")
	
{

}