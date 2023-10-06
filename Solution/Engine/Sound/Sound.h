#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <xaudio2.h>

class SoundData;

class Sound
{
private:
	Sound(const Sound& sd) = delete;
	Sound& operator=(const Sound& sd) = delete;

	Sound() = default;

	std::unordered_map<std::string, std::shared_ptr<SoundData>> datas;

public:
	enum class WAVEFORM : uint8_t
	{
		SAWTOOTH,	// のこぎり波
		SIN,		// 正弦波
		SQUARE		// 矩形波
	};

public:
	~Sound();

	static inline Sound* getInstance()
	{
		static Sound sound{};
		return &sound;
	}
	static inline Sound* ins() { return getInstance(); }

	std::weak_ptr<SoundData> loadWave(const std::string& filePath);

	std::weak_ptr<SoundData> loadWave(WAVEFORM waveform, float hz, float sec = 1.f);

	/// @brief 音を再生
	/// @param data 再生するデータ
	/// @param loopCount 0で繰り返し無し、XAUDIO2_LOOP_INFINITEで永遠
	/// @param volume 0~1
	/// @return 異常があればtrue
	static bool playWave(std::weak_ptr<SoundData>& data, uint32_t loopCount = 0u, float volume = 0.2f);

	/// @brief 音を止める
	/// @param data 音データ
	/// @return 異常があればtrue
	static bool stopWave(std::weak_ptr<SoundData>& data);
};
