/**
 * @file Timer.cpp
 * @brief 時間取得クラス
 */

#include "Timer.h"

using namespace std::chrono;

Timer::Timer() :
	startTime(steady_clock::now())
{}