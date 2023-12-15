#include "TutorialTexture.h"

#include<DirectXMath.h>
#include<3D/Billboard/Billboard.h>
#include<GameCamera.h>

using namespace DirectX;

namespace
{
	const wchar_t* texPath = L"Resources/Map/Tex/blockobj.png";
};

TutorialTexture::TutorialTexture(GameCamera* camera,const unsigned short stageNum)
	:gameObj(std::make_unique<Billboard>(texPath, camera))
	,camera(camera)
	, STAGE_NUM(stageNum)
{
	// 数値は仮設定
	gameObj->add(XMFLOAT3(300, -700,0), 300.f);

	createState();
}

void TutorialTexture::update()
{
	gameObj->update(XMConvertToRadians(-camera->getAngleDeg()));
	state->update();
}

void TutorialTexture::draw()
{
	gameObj->draw();
}

void TutorialTexture::createState()
{
	constexpr short TUTORIAL_MOVE = 0;
	constexpr short TUTORIAL_JUMP = 1;

	if(STAGE_NUM == TUTORIAL_MOVE)
	{
		state = std::make_unique<Move>(gameObj.get());
	}
	else 
	if(STAGE_NUM == TUTORIAL_JUMP)
	{
		state = std::make_unique<Jump>(gameObj.get());
	}
}

void TutorialTexture::Move::update()
{
	// 回転させる
}

void TutorialTexture::Jump::update()
{
	// 思いっきり振り上げ、ゆっくり下に戻す処理
}
