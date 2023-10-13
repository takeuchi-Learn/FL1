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
#include "GameClear.h"
#include "GameOver.h"
using namespace DirectX;

namespace
{
	inline auto toImVec2(const XMFLOAT2& f)
	{
		return ImVec2(f.x, f.y);
	}

	constexpr XMFLOAT3 objectPosDef = XMFLOAT3(0, 0, 0);
	constexpr XMFLOAT3 cameraPosDef = XMFLOAT3(0, 0, -5);
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
	camera(std::make_unique<Camera>((float)WinAPI::window_width,
									(float)WinAPI::window_height)),
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

	light->setPointLightActive(0u, true);
	light->setPointLightPos(0u, lightPosDef);
	defAtt = light->getPointLightAtten(0u);
	pLightPolarPos = lightPosDef;

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
	if (Input::ins()->triggerKey(DIK_1))
	{
		SceneManager::ins()->changeScene<GameClear>();
		return;
	}
	if (Input::ins()->triggerKey(DIK_2))
	{
		SceneManager::ins()->changeScene<GameOver>();
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

	// 毎フレーム頂点を移動する
	// 法線のスムーシングには非対応
	if (isPlayAnim)
	{
		// 五乗
		//const float rate = easeInOutCubic(float(frame) / (float)frameMax);

		// 線形
		const float rate = (float)frame / (float)frameMax;

		const auto& start = startModel->getMesh();
		const auto& end = endModel->getMesh();
		const auto& nowModel = model->getMesh();

		const auto len = std::min({ start.size(), end.size(), nowModel.size() });

		for (size_t mesh = 0u; mesh < len; ++mesh)
		{
			// 頂点
			{
				auto& startVert = start[mesh]->getVertices();
				auto& endVert = end[mesh]->getVertices();
				auto& nowVert = nowModel[mesh]->getVertices();

				const auto meshLen = std::min({ startVert.size(), endVert.size(), nowVert.size() });
				for (size_t i = 0u; i < meshLen; ++i)
				{
					nowVert[i].pos = lerp(startVert[i].pos,
										  endVert[i].pos,
										  rate);
					nowVert[i].normal = lerp(startVert[i].normal,
											 endVert[i].normal,
											 rate);
					nowVert[i].uv = lerp(startVert[i].uv,
										 endVert[i].uv,
										 rate);
				}
				nowModel[mesh]->transferVertBuf();
			}

			// インデックス
			{
				auto& startIndex = start[mesh]->getIndices();
				auto& endIndex = end[mesh]->getIndices();
				auto& nowIndex = nowModel[mesh]->getIndices();

				const auto indexLen = std::min({ startIndex.size(), endIndex.size(), nowIndex.size() });
				for (size_t i = 0u; i < indexLen; ++i)
				{
					nowIndex[i] = (unsigned short)std::lerp(startIndex[i], endIndex[i], rate);
				}
				nowModel[mesh]->transferIndexBuf();
			}
		}

		// 最後まで行ったら始点と終点を入れ替えてもう一度
		if (++frame >= frameMax)
		{
			std::swap(startModel, endModel);
			frame = 0ui16;
		}
	}

	if (Input::ins()->triggerKey(DIK_0))
	{
		camera->setEye(cameraPosDef);
		if (light->getPointLightActive(0u))
		{
			light->setPointLightPos(0u, camera->getEye());
		}
		pointLightPolarPos = cameraPosDef;
	} else
	{
		const bool hitW = Input::ins()->hitKey(DIK_W);
		const bool hitA = Input::ins()->hitKey(DIK_A);
		const bool hitS = Input::ins()->hitKey(DIK_S);
		const bool hitD = Input::ins()->hitKey(DIK_D);

		if (hitW || hitA || hitS || hitD)
		{
			constexpr float speed = XM_PI / 100.f;

			if (hitW || hitS)
			{
				if (hitW)
				{
					pointLightPolarPos.thetaRad -= speed;
					if (pointLightPolarPos.thetaRad < 0.f)
					{
						pointLightPolarPos.thetaRad += XM_2PI;
					}
				}
				if (hitS)
				{
					pointLightPolarPos.thetaRad += speed;
					if (pointLightPolarPos.thetaRad > XM_2PI)
					{
						pointLightPolarPos.thetaRad -= XM_2PI;
					}
				}
			}

			if (hitA || hitD)
			{
				if (hitA)
				{
					pointLightPolarPos.phiRad -= speed;
					if (pointLightPolarPos.phiRad < 0.f)
					{
						pointLightPolarPos.phiRad += XM_2PI;
					}
				}
				if (hitD)
				{
					pointLightPolarPos.phiRad += speed;
					if (pointLightPolarPos.phiRad > XM_2PI)
					{
						pointLightPolarPos.phiRad -= XM_2PI;
					}
				}
			}

			light->setPointLightPos(0u, pointLightPolarPos.toXMFLOAT3());
		}
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

	using namespace ImGui;

	SetNextWindowPos(ImVec2(0.f, 0.f));

	Begin("PlaySceneGUI", nullptr, DX12Base::imGuiWinFlagsDef);
	Text("タイマー%s: %f",
		 stopwatch->isPlay() ? "|>" : "□",
		 float(stopwatchPlayTime) / Timer::oneSecF);
	{
		float alpha = object->color.w;
		if (SliderFloat("透明度", &alpha, 0.f, 1.f))
		{
			object->color.w = alpha;
		}
	}
	Text("スペース: シーン切り替え");
	Text("x:%.1f, y:%.1f, z:%.1f",
		camera->getEye().x,
		camera->getEye().y,
		camera->getEye().z);
	Text("x:%.1f, y:%.1f, z:%.1f",
		light->getPointLightPos(0u).x,
		light->getPointLightPos(0u).y,
		light->getPointLightPos(0u).z);
	if (RadioButton("アニメーション再生", isPlayAnim))
	{
		isPlayAnim = !isPlayAnim;
	}
	if (RadioButton("点光源", light->getPointLightActive(0u)))
	{
		light->setPointLightActive(0u, !light->getPointLightActive(0u));
	}
	if (RadioButton("平行光源", light->getDirLightActive(0u)))
	{
		light->setDirLightActive(0u, !light->getDirLightActive(0u));
	}
	if (light->getDirLightActive(0u))
	{
		const auto& dirVec = light->getDirLightDir(0u);

		XMFLOAT3 dirF3{};
		XMStoreFloat3(&dirF3, dirVec);

		Polar dirPolar = dirF3;

		float rad[2] = { dirPolar.thetaRad, dirPolar.phiRad };

		bool dirty = false;
		if (SliderFloat("θtheta", &rad[0], -XM_PI, XM_PI))
		{
			if (!std::isfinite(rad[0]) || rad[0] == 0.f)
			{
				rad[0] = XM_PI;
			}
			dirPolar.thetaRad = rad[0];
			dirty = true;
		}
		if (SliderFloat("φphi", &rad[1], 0.f, XM_2PI) || rad[1] <= 0.f)
		{
			if (!std::isfinite(rad[1]))
			{
				rad[1] = XM_2PI;
			}
			dirPolar.phiRad = rad[1];
			dirty = true;
		}
		if (dirty)
		{
			light->setDirLightDir(0u, XMLoadFloat3(&dirPolar.toXMFLOAT3()));
		}
	}

	{
		const auto& modelMat = model->getOneMaterialItr()->second;

		{
			float metal = modelMat->ambient.x;
			if (SliderFloat("metallic", &metal, 0.f, 1.f))
			{
				model->setAmbient(XMFLOAT3(metal, metal, metal));
			}
		}
		{
			float roughness = modelMat->diffuse.x;
			if (SliderFloat("roughness", &roughness, 0.f, 1.f))
			{
				model->setDiffuse(XMFLOAT3(roughness, roughness, roughness));
			}
		}
		{
			float specular = modelMat->specular.x;
			if (SliderFloat("specular", &specular, 0.f, 1.f))
			{
				model->setSpecular(XMFLOAT3(specular, specular, specular));
			}
		}
		Text("マテリアル%u個", model->getMaterial().size());
	}
	{
		static float tmp[3]{ object->color.x, object->color.y, object->color.z };
		if (ColorPicker3("色", tmp))
		{
			object->color = XMFLOAT4(tmp[0], tmp[1], tmp[2], 1.f);
		}
	}
	End();
}