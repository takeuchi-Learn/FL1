/**
 * @file Timer.h
 * @brief 時間取得クラス
 */

#pragma once

#include <chrono>

 /// @brief 時間取得クラス
class Timer
{
protected:
	std::chrono::steady_clock::time_point  startTime{};

public:
	float bpm = 100.f;

	Timer();
	~Timer() = default;

public:
	/// @brief 時間の単位
	using timeUnit = std::chrono::microseconds;
	/// @brief 時間を格納する型
	using timeType = long long;

	/// @brief 一秒
	static constexpr timeType oneSec = std::chrono::duration_cast<timeUnit>(std::chrono::seconds(1)).count();
	/// @brief 一秒float型
	static constexpr float oneSecF = static_cast<float>(oneSec);

	static constexpr timeType oneMin = oneSec * static_cast<timeType>(60);
	static constexpr float oneMinF = static_cast<float>(oneMin);

	/// @brief 一拍の時間を取得
	/// @param bpm 一分間の拍数
	/// @return 一拍の時間
	template <class floatType = double>
	static constexpr timeType calc1BeatTime(floatType bpm)
	{
		return timeType((floatType)std::chrono::duration_cast<timeUnit>(std::chrono::seconds(60ll)).count() / bpm);
	};

	/// @brief エポックからの経過時間を取得
	/// @return 経過時間
	static inline timeType getNowTimeSinceEpoch()
	{
		return std::chrono::duration_cast<timeUnit>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	/// @brief 判定関数
	/// @param judgeBeatRaito 判定する拍内進行度 [0~1]
	/// @param okRange 成功とする範囲 [0~1]
	/// @return trueで成功
	static constexpr bool judge(float judgeBeatRaito, float okRange = 0.25f)
	{
		// judgeBeatRaitoと0.5の差を求める(0 ~ 0.5)
		const float diff =
			judgeBeatRaito > 0.5f ?
			judgeBeatRaito - 0.5f :
			0.5f - judgeBeatRaito;

		// 失敗の範囲(0 ~ 1)
		const float noRange = 1.f - okRange;

		// 差が既定の値より大きければよい
		return diff * 2.f > noRange;
	}

	/// @brief 現在の拍内での進行度を算出
	/// @param nowTime 現在の時間
	/// @param bpm 曲のbpm
	/// @param nowCount 現在の拍数出力用(float)
	/// @return 現在の拍内での進行度
	static inline float calcNowBeatRaito(float nowTime, float bpm, float& nowCountBuf)
	{
		return std::modf(nowTime / float(calc1BeatTime(bpm)), &nowCountBuf);
	}

	/// @brief 起点時間から現在までの時間を取得
	/// @return 起点時間から現在までの時間
	inline timeType getNowTime() const
	{
		return std::chrono::duration_cast<timeUnit>(std::chrono::steady_clock::now() - startTime).count();
	}

	/// @brief 現在を起点時間とする(現在を0とする)
	inline void reset() { startTime = std::chrono::steady_clock::now(); }
};
