#include "Sound.h"
#include "SoundData.h"
#include <cassert>
#include <fstream>

Sound::~Sound()
{
	datas.clear();
}

std::weak_ptr<SoundData> Sound::loadWave(const std::string& filePath)
{
	std::shared_ptr<SoundData>& ret = datas[filePath];

	// 重複防止
	if (ret)
	{
		return ret;
	}

	ret = std::make_shared<SoundData>(filePath.c_str());

	return ret;
}

std::weak_ptr<SoundData> Sound::loadWave(WAVEFORM waveform, float hz, float sec)
{
	std::shared_ptr<SoundData>& ret = datas["/\\." + std::to_string(hz) + std::to_string((uint8_t)waveform)];

	// 重複防止
	if (ret)
	{
		return ret;
	}

	ret = std::make_shared<SoundData>(waveform, hz, sec);

	return ret;
}

bool Sound::playWave(std::weak_ptr<SoundData>& data, uint32_t loopCount, float volume)
{
	if (data.expired()) { return true; }
	SoundData::SoundPlayWave(data.lock().get(), loopCount, volume);
	return false;
}

bool Sound::stopWave(std::weak_ptr<SoundData>& data)
{
	if (data.expired()) { return true; }
	SoundData::SoundStopWave(data.lock().get());
	return false;
}