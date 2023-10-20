#include "PlayScene.h"
#include <System/DX12Base.h>
#include <System/SceneManager.h>
#include <Input/Input.h>
#include <Camera/Camera.h>
#include <3D/Light/Light.h>
#include <3D/Obj/Object3d.h>
#include <3D/Obj/ObjModel.h>
#include <2D/Sprite.h>
#include <Sound/Sound.h>
#include <Sound/SoundData.h>
#include <3D/ParticleMgr.h>
#include <algorithm>

#include "TitleScene.h"

using namespace DirectX;

namespace
{
	inline auto toImVec2(const XMFLOAT2& f)
	{
		return ImVec2(f.x, f.y);
	}

	constexpr XMFLOAT3 objectPosDef = XMFLOAT3(0, 0, 0);
	constexpr XMFLOAT3 cameraPosDef = XMFLOAT3(0, 0, -600);
	constexpr XMFLOAT3 lightPosDef = XMFLOAT3(0, 0, -cameraPosDef.z * 2.f);

	XMFLOAT3 defAtt{};

	bool isPlayAnim{};

	constexpr uint16_t frameMax = 120ui16;
	uint16_t frame{};

	std::unique_ptr<ObjModel> startModel, endModel;

	constexpr XMFLOAT2 lerp(const XMFLOAT2& s, const XMFLOAT2& e, float t)
	{
		return XMFLOAT2(
			std::lerp(s.x, e.x, t),
			std::lerp(s.y, e.y, t)
		);
	}
	constexpr XMFLOAT3 lerp(const XMFLOAT3& s, const XMFLOAT3& e, float t)
	{
		return XMFLOAT3(
			std::lerp(s.x, e.x, t),
			std::lerp(s.y, e.y, t),
			std::lerp(s.z, e.z, t)
		);
	}

	constexpr float easeInOutCubic(float x)
	{
		if (x < 0.5f)
		{
			return 4.f * x * x * x;
		}
		const float n = (-2.f * x + 2.f) / 2.f;
		return 1.f - n * n * n;
	}

	struct Polar
	{
		float r;
		float thetaRad;
		float phiRad;

		Polar(float r, float thetaRad, float phiRad) :
			r(r), thetaRad(thetaRad), phiRad(phiRad)
		{}

		inline Polar() : Polar(1.f, 0.f, 0.f) {}

		Polar(const XMFLOAT3& f) :
			r(std::sqrt(f.x* f.x + f.y * f.y + f.z * f.z)),
			phiRad(std::atan(f.z / (f.x != 0.f ? f.x : 1.E38f)))
		{
			thetaRad = std::acos(f.y / r);
		}

		static inline XMFLOAT3 transF3(const Polar& p)
		{
			const float rSinT = p.r * std::sin(p.thetaRad);
			return XMFLOAT3(rSinT * std::cos(p.phiRad),
							p.r * std::cos(p.thetaRad),
							rSinT * std::sin(p.phiRad));
		}

		inline XMFLOAT3 toXMFLOAT3() const { return transF3(*this); }
	};

	Polar pointLightPolarPos{}, pLightPolarPos{};
}

PlayScene::PlayScene() :
	light(std::make_unique<Light>()),
	camera(std::make_unique<GameCamera>()),
	stopwatch(std::make_unique<Stopwatch>()),
	stopwatchPlayTime(Timer::timeType(0u))
{
	bgm = Sound::ins()->loadWave("Resources/BGM.wav");

	spriteBase = std::make_unique<SpriteBase>();
	sprite = std::make_unique<Sprite>(spriteBase->loadTexture(L"Resources/judgeRange.png"),
									  spriteBase.get(),
									  XMFLOAT2(0.5f, 0.5f));
	sprite->color.w = 0.5f;

	camera->setEye(cameraPosDef);
	camera->setTarget(objectPosDef);
	pointLightPolarPos = cameraPosDef;

	constexpr float lightAmbient = 0.f;
	light->setAmbientColor(XMFLOAT3(lightAmbient, lightAmbient, lightAmbient));

	light->setDirLightActive(0u, true);
	light->setDirLightDir(0u, XMVectorSet(1, 0, 0, 0));
	light->setPointLightPos(0u, camera->getEye());

	isPlayAnim = false;
	frame = 0ui16;

	pbrPP = Object3d::createGraphicsPipeline(BaseObj::BLEND_MODE::ALPHA,
											 L"Resources/Shaders/PbrObjVS.hlsl",
											 L"Resources/Shaders/PbrObjPS.hlsl");
	model = std::make_unique<ObjModel>("Resources/sayuu", "sayuu", 0u, true, true);
	startModel = std::make_unique<ObjModel>("Resources/sayuu", "sayuu", 0u, true, true);
	endModel = std::make_unique<ObjModel>("Resources/sayuu", "sayuu2", 0u, true, true);
	object = std::make_unique<Object3d>(camera.get(), model.get());
	object->position = objectPosDef;
	object->scale = { 100.f,100.f,100.f };
	object->color = XMFLOAT4(0.25f, 0.25f, 1, 1);

	particle = std::make_unique<ParticleMgr>(L"Resources/judgeRange.png", camera.get());

	Sound::ins()->playWave(bgm,
						   XAUDIO2_LOOP_INFINITE,
						   0.2f);
}

PlayScene::~PlayScene()
{
	Sound::ins()->stopWave(bgm);
}

void PlayScene::update()
{
	// スペースでシーン切り替え
	if (Input::ins()->triggerKey(DIK_SPACE))
	{
		SceneManager::ins()->changeScene<TitleScene>();
		return;
	}

	if (Input::ins()->triggerKey(DIK_P))
	{
		constexpr float particleVel = 0.1f;
		particle->createParticle(object->position,
								 10u,
								 1.f,
								 particleVel);
	}

	if (Input::ins()->triggerKey(DIK_B))
	{
		static bool isPlay = true;

		if (isPlay)
		{
			bgm.lock()->stopVoice();
		} else
		{
			bgm.lock()->startVoice();
		}
		isPlay = !isPlay;
	}

	// Tを押したらタイマーを停止/再開
	if (Input::ins()->triggerKey(DIK_T))
	{
		stopwatch->stopOrResume();
	}
	// 測定中なら現在時間を更新
	if (stopwatch->isPlay())
	{
		stopwatchPlayTime = stopwatch->getNowTime();
	}

	// ライトとカメラの更新
	camera->update();
	camera->gameCameraUpdate();
	light->update();

}

void PlayScene::drawObj3d()
{
	object->drawWithUpdate(light.get(), pbrPP);
	particle->drawWithUpdate();
}

void PlayScene::drawFrontSprite()
{
	const auto mousePos = Input::ins()->calcMousePosFromSystem();
	sprite->position.x = mousePos.x;
	sprite->position.y = mousePos.y;

	spriteBase->drawStart(DX12Base::ins()->getCmdList());
	sprite->drawWithUpdate(DX12Base::ins(), spriteBase.get());
}