/*****************************************************************//**
 * \file   TutorialTexture.h
 * \brief  チュートリアルの画像
 * \note   アニメーションさせるために複数画像用意するの大変だと思うので、プログラムで回転させたり上下に動かしたほうがよさそう
 *********************************************************************/

#pragma once
#include <memory>
#include <cmath>
#include <DirectXMath.h>
#include <Util/Stopwatch.h>

class Billboard;
class GameCamera;
class Stopwatch;

// 仕様
// 個数少ないので、プレイヤーと同じ配置方法を想定して作成
// プレイヤーと同じ方法で作成するが、それだと画像を正方形にしないといけない
// 画像が長方形の場合は、空白増やして正方形にすること

/// @brief 操作説明の画像
class TutorialTexture
{
private:
	class State
	{
	protected:
		Billboard* gameObj = nullptr;
		GameCamera* camera = nullptr;
		std::unique_ptr<Stopwatch> timer;
		const Timer::timeType interval;
		float raito = 0.f;

	public:
		State(Billboard* gameObj, GameCamera* camera, Timer::timeType interval);

		void updateCount();

		virtual void update() {}
	};

	class RotationTutorial :public State
	{
	public:
		RotationTutorial(Billboard* gameObj, GameCamera* camera)
			: State(gameObj, camera, 2u * Timer::oneSec)
		{}

		void update() override;
	};

	class JumpTutorial :public State
	{
	private:
		// 上げ用 easeOutQuint
		constexpr float easingUp(const float x)
		{
			const float value = 1.f - x;
			const float valuePow5 = value * value * value * value * value;
			return 1.f - valuePow5;
		}

		// 下げ用 easeInOutSine
		inline float easingDown(const float x)
		{
			return -(std::cos(x * DirectX::XM_PI) - 1.f) / 2.f;
		}

		// 初期位置
		const float START_POS_Y;

	public:
		JumpTutorial(Billboard* gameObj, GameCamera* camera);
		void update() override;
	};

	std::unique_ptr<State> state;
	const uint16_t STAGE_NUM;

private:
	std::unique_ptr<Billboard> gameObj;
	GameCamera* camera = nullptr;

private:
	void createState();

public:
	TutorialTexture(GameCamera* camera, uint16_t stageNum);
	~TutorialTexture() {}

	void update();
	void draw();

	/// @brief ステージ選択での調整用
	void setPosition(const DirectX::XMFLOAT2& pos);
	/// @brief ステージ選択での調整用
	void setScale(float scale);
};
