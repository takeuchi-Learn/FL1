#include "TutorialTexture.h"
#include <3D/Billboard/Billboard.h>
#include <GameCamera.h>

using namespace DirectX;

namespace
{
	const wchar_t* texPath = L"Resources/Map/Tex/blockobj.png";
};

TutorialTexture::TutorialTexture(GameCamera* camera, const uint16_t stageNum)
	:gameObj(std::make_unique<Billboard>(texPath, camera))
	, camera(camera)
	, STAGE_NUM(stageNum)
{
	// 数値は仮設定
	gameObj->add(XMFLOAT3(300, -700, 0), 300.f);

	createState();
}

void TutorialTexture::update()
{
	state->update();
	state->updateCount();
}

void TutorialTexture::draw()
{
	gameObj->draw();
}

void TutorialTexture::createState()
{
	constexpr decltype(STAGE_NUM) TUTORIAL_MOVE = 0ui16;
	constexpr decltype(STAGE_NUM) TUTORIAL_JUMP = 1ui16;

	switch (STAGE_NUM)
	{
	case TUTORIAL_MOVE:
		state = std::make_unique<RotationTutorial>(gameObj.get(), camera);
		break;
	case TUTORIAL_JUMP:
		state = std::make_unique<JumpTutorial>(gameObj.get(), camera);
		break;
	}
}

void TutorialTexture::RotationTutorial::update()
{
	// 回転させる

	constexpr float angleMax = 45.f;
	const float angleDeg = std::sin(raito * XM_2PI) * angleMax;

	gameObj->update(XMConvertToRadians(angleDeg - camera->getAngleDeg()));

	// 終了処理 何かしらあったら記述
	/*if (loopEnd())
	{
	}*/
}

TutorialTexture::JumpTutorial::JumpTutorial(Billboard* gameObj, GameCamera* camera)
	:State(gameObj, camera, Timer::oneMS * 2500u)
	, START_POS_Y(gameObj->getFrontData()->position.y)
{}

void TutorialTexture::JumpTutorial::update()
{
	auto& me = gameObj->getFrontData();

	// 上昇幅
	constexpr float moveValMax = 100.f;

	// 上昇から下降に変わる閾値
	constexpr float thresholdRaito = 0.2f;
	if (raito < thresholdRaito)
	{
		// 上がる
		const float t = raito / thresholdRaito;
		me->position.y = moveValMax * easingUp(t);
	} else
	{
		// 下がる
		constexpr float inv = 1.f - thresholdRaito;
		const float t = 1.f - ((raito - thresholdRaito) / inv);
		me->position.y = moveValMax * easingDown(t);
	}

	me->position.y += START_POS_Y;

	gameObj->update(XMConvertToRadians(-camera->getAngleDeg()));
}

TutorialTexture::State::State(Billboard* gameObj,
							  GameCamera* camera,
							  Timer::timeType interval)
	: gameObj(gameObj)
	, camera(camera)
	, timer(std::make_unique<Stopwatch>())
	, interval(interval)
{
	timer->reset();
}

void TutorialTexture::State::updateCount()
{
	const auto now = timer->getNowTime();
	if (now >= interval)
	{
		raito = 0.f;
		timer->reset();
		return;
	}
	raito = static_cast<float>(now) / static_cast<float>(interval);
}
