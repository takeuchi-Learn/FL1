/*****************************************************************//**
 * \file   Stopwatch.h
 * \brief  Timerクラスに一時停止機能を追加したもの
 *********************************************************************/

#pragma once

#include "Timer.h"

 /// @brief Timerクラスに一時停止機能を追加したもの
class Stopwatch :
	public Timer
{
public:
	using Timer::Timer;

private:
	std::chrono::steady_clock::duration pauseTime{};
	bool playFlag = true;

public:
	inline bool isPlay() const { return playFlag; }

	/// @brief 停止
	inline void stop()
	{
		pauseTime = std::chrono::steady_clock::now() - startTime;
		playFlag = false;
	}

	/// @brief 再開
	inline void resume()
	{
		startTime = std::chrono::steady_clock::now() - pauseTime;
		playFlag = true;
	}

	/// @brief 測定中なら停止、停止中なら再開する。if文で分岐しているだけ。
	inline void stopOrResume()
	{
		if (playFlag)
		{
			stop();
		} else
		{
			resume();
		}
	}
};
