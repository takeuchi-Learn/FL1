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
	state->update();
	state->addTime();
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
		state = std::make_unique<Move>(gameObj.get(), camera);
	}
	else 
	if(STAGE_NUM == TUTORIAL_JUMP)
	{
		state = std::make_unique<Jump>(gameObj.get(), camera);
	}
}

void TutorialTexture::Move::update()
{

	// 回転させる

	constexpr float piMag2 = 3.14f * 2;
	if (x < piMag2)
	{
		constexpr float frameAddNum = 0.05f;
		x += frameAddNum;
	}
	else
	{
		x = piMag2;
	}

	constexpr float mag = 45.f;
	float angleDeg = std::sin(x);
	angleDeg *= mag;
	gameObj->getFrontData()->rotation = angleDeg;

	gameObj->update(XMConvertToRadians(-camera->getAngleDeg() + angleDeg));
	
	// 終了処理 何かしらあったら記述
	if(loopEnd())
	{
	}

}

TutorialTexture::Jump::Jump(Billboard* gameObj, GameCamera* camera)
	:State(gameObj, camera, static_cast<unsigned int>(60.f * 2.5f))
	, START_POS_Y(gameObj->getFrontData()->position.y)
{
}

void TutorialTexture::Jump::update()
{
	constexpr unsigned int startTime = static_cast<unsigned int>(60.f * 1.f);
	if (getTime() >= startTime)jump();

	gameObj->update(XMConvertToRadians(-camera->getAngleDeg()));

	// 終了処理
	if (loopEnd())
	{
		gameObj->getFrontData()->position.y = START_POS_Y; 
		downMove = false;
	}

}

void TutorialTexture::Jump::jump()
{
	float& posY = gameObj->getFrontData()->position.y;

	constexpr float mag = 200.f;
	if (downMove)
	{
		posY = START_POS_Y + easingDown(1.f - x) * mag;
	}
	else
	{
		posY = START_POS_Y + easingUp(x) * mag;
	}

	if (x < 1.f)
	{
		constexpr float frameAddNum = 0.04f;
		x += frameAddNum;
	}
	else
	{
		if (!downMove)
		{
			x = 0.f;
			downMove = true;
		} 
		else
		{
			x = 1.f;
		}
	}
}

bool TutorialTexture::State::loopEnd()
{
	if(time == TIME_MAX)
	{
		time = 0;
		x = 0;
		return true;
	}
	return false;
}
