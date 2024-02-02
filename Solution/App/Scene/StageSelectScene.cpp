﻿#include "StageSelectScene.h"
#include "PlayScene.h"
#include <System/Dx12Base.h>
#include <Input/Input.h>
#include <Input/PadImu.h>
#include <2D/Sprite.h>
#include <2D/SpriteBase.h>
#include <Imu/Sensor.h>
#include <System/SceneManager.h>
#include <Util/Stopwatch.h>
#include <Util/Util.h>
#include<GameCamera.h>
#include <filesystem>
#include <format>
#include<TutorialTexture.h>

using namespace DirectX;

namespace
{
	constexpr auto transitionTime = Timer::oneSec * 1.5;

	inline bool stageExist(uint16_t stageNum)
	{
		const auto mapYamlPath = "Resources/Map/map_" + std::to_string(stageNum) + ".yml";
		return std::filesystem::exists(mapYamlPath);
	}
}

StageSelectScene::StageSelectScene() :
	timer(std::make_unique<Stopwatch>())
{
	// ステージ存在確認
	for (uint16_t i = 0u; i < UINT16_MAX; ++i)
	{
		if (!stageExist(i))
		{
			stageMaxNum = i == 0u ? 0u : i - 1u;
			break;
		}
	}
	currentStage = std::clamp(PlayScene::getStageNum(), 0ui16, stageMaxNum);

	update_proc = std::bind(&StageSelectScene::update_main, this);


	// テクスチャ準備
	spBase = std::make_unique<SpriteBase>();
	// 背景
	backGroundSprite = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/stageSelect/stageSelectBG.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	constexpr XMFLOAT2 winSize = XMFLOAT2(float(WinAPI::window_width), float(WinAPI::window_height));
	backGroundSprite->setSize(winSize);

	// 矢印
	arrowSprite = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/stageSelect/arrow.png"), spBase.get(), XMFLOAT2(0.f, 0.f));
	constexpr float arrowSize = 1200.f;
	arrowSprite->setSize(XMFLOAT2(arrowSize, arrowSize));
	arrowSprite->position = XMFLOAT3(WinAPI::window_width / 2.f, 450.f, 0.f);
	arrowSprite->setAnchorPoint(XMFLOAT2(0.5f, 0.5f));

	// ステージ画像
	stageTexSprite.resize(stageMaxNum + 1);
	for (uint16_t i = 0u; i < stageMaxNum + 1; i++)
	{

		const std::wstring path = L"Resources/stageSelect/stage" + std::to_wstring(i) + L".png";
		stageTexSprite[i] = std::make_unique<Sprite>(spBase->loadTexture(path.c_str()), spBase.get(), XMFLOAT2(0.f, 0.f));
		stageTexSprite[i]->setAnchorPoint(XMFLOAT2(0.5f, 0.5f));

	}

	// 操作方法表示で使う使うカメラ
	camera = std::make_unique<GameCamera>();
	// 操作方法
	tutorialTexture = std::make_unique<TutorialTexture>(camera.get(), 0);
	tutorialTexture->setPosition(XMFLOAT2(0.0f, 8.0f));
	tutorialTexture->setScale(6.f);
}

void StageSelectScene::update_main()
{
	bool inputR = Input::ins()->triggerKey(DIK_RIGHT);
	inputR |= Input::ins()->triggerPadButton(Input::PAD::PADNUM::RIGHT);

	bool inputL = Input::ins()->triggerKey(DIK_LEFT);
	inputL |= Input::ins()->triggerPadButton(Input::PAD::PADNUM::LEFT);

	if (PadImu::ins()->getDevCount() > 0)
	{
		inputR |= PadImu::ins()->checkTriggerInputDPAD(0, PadImu::DIRECTION::RIGHT);
		inputL |= PadImu::ins()->checkTriggerInputDPAD(0, PadImu::DIRECTION::LEFT);
	}

	if (inputR)
	{
		if (currentStage < stageMaxNum) { ++currentStage; }
	} else if (inputL)
	{
		if (currentStage > 0u) { --currentStage; }
	} else if (PadImu::ins()->checkInputAccept() || Sensor::ins()->CheckButton())
	{
		PlayScene::setStageNum(currentStage);
		thread = std::make_unique<std::jthread>([&]
												{
													nextScene = std::make_unique<PlayScene>();
												});
		update_proc = std::bind(&StageSelectScene::update_transition, this);
		timer->reset();
	}

	// 操作方法更新
	tutorialTexture->update();
}

void StageSelectScene::update_transition()
{
	const auto nowTime = timer->getNowTime();
	if (nowTime >= transitionTime)
	{
		transitionRaito = 1.f;
		thread->join();
		SceneManager::ins()->changeSceneFromInstance(nextScene);
		return;
	}

	transitionRaito = static_cast<float>(nowTime) / static_cast<float>(transitionTime);
}

void StageSelectScene::update()
{
	update_proc();
}

void StageSelectScene::drawFrontSprite()
{

	spBase->drawStart(DX12Base::ins()->getCmdList());
	backGroundSprite->drawWithUpdate(DX12Base::ins(), spBase.get());
	arrowSprite->drawWithUpdate(DX12Base::ins(), spBase.get());

	using namespace ImGui;

	const auto stageCount = stageMaxNum + 1;
	for (int i = 0; i < stageCount; ++i)
	{
		// todo GameMapのstatic定数にする
		constexpr float mapSize = 100.f;
		//constexpr float sizeMax = mapSize * 2.f;
		constexpr float sizeMax = mapSize * 3.2f;

		{
			const float shiftVal = mapSize * 1.5f;
			constexpr auto posY = static_cast<float>(WinAPI::window_height) / 2.f;
			constexpr auto posX = static_cast<float>(WinAPI::window_width) / 2.f;

			const auto windowPos = ImVec2(posX + (int(i) - int(currentStage)) * sizeMax, posY);
			SetNextWindowPos(windowPos, 0, ImVec2(0.5f, 0.5f));

			if (i <= stageMaxNum)
			{
				if (stageTexSprite[i])
				{
					stageTexSprite[i]->position.x = windowPos.x;
					stageTexSprite[i]->position.y = windowPos.y;
				}
			}
		}

		{
			float raito = static_cast<float>(stageCount - std::abs(int(currentStage) - int(i))) / static_cast<float>(stageCount);
			raito = 1.f - raito;
			raito = 1.f - raito * raito;
			raito = std::clamp(raito, 0.f, 1.f);

			float size = -1.f;
			if (i == currentStage)
			{
				// サイズは最大
				constexpr float transitionEndSize = static_cast<float>(1 + std::max(WinAPI::window_width, WinAPI::window_height));
				size = std::lerp(sizeMax, transitionEndSize, Util::easeOutBounce(transitionRaito));
				// フォーカスはこのウインドウに合わせる（演出時に最前面にするため）
				SetNextWindowFocus();
			} else
			{
				constexpr auto min = sizeMax * 0.125f;
				constexpr auto max = sizeMax * 0.875f;
				size = std::lerp(min, max, raito);
			}

			SetNextWindowSize(ImVec2(size, size));

			if (i <= stageMaxNum)
			{
				// stage3の画像が無いのでif書いてます
				if (stageTexSprite[i])
				{
					if (i == currentStage)
					{
						const float sprSize = size * 0.8f;
						stageTexSprite[i]->setSize(XMFLOAT2(sprSize, sprSize));
					}
					else
					{
						const float sprSize = size * 0.82f;
						stageTexSprite[i]->setSize(XMFLOAT2(sprSize, sprSize));
					}
				}
			}
		}


		Begin(std::format("StageSelectScene::drawFrontSprite{}", i).c_str(), nullptr,
			  DX12Base::imGuiWinFlagsNoTitleBar );
		//Text("\n");
		//Text(std::format("{}", i).c_str());
		//SetWindowFontScale(1.5f);
		End();
	}

	DX12Base::getInstance()->endImGui();
	DX12Base::getInstance()->startImGui();


	spBase->drawStart(DX12Base::ins()->getCmdList());
	for (int i = 0; i < stageCount; ++i)
	{
		if (i <= stageMaxNum)
		{
			// 選択してるやつだけ一番上に描画するのでcontinue
			if (i == currentStage)continue;

			// stage3の画像が無いのでif書いてます
			if (stageTexSprite[i])
			{
				stageTexSprite[i]->drawWithUpdate(DX12Base::ins(), spBase.get());
			}
		}
	}
	tutorialTexture->draw();


	spBase->drawStart(DX12Base::ins()->getCmdList());
	// 一番上に表示したいのでここで呼び出し
	if (stageTexSprite[currentStage])
	{
		stageTexSprite[currentStage]->drawWithUpdate(DX12Base::ins(), spBase.get());
	}
}