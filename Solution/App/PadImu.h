#pragma once

#include <vector>

class PadImu
{
private:
	PadImu(const PadImu&) = delete;
	PadImu& operator=(const PadImu&) = delete;
	PadImu();

private:
	std::vector<int> devHandles{};
	int devCount = -1;

	// コンストラクタ内部で呼んでいる
	void init();

public:
	inline static PadImu* ins()
	{
		static PadImu imu{};
		return &imu;
	}

	void reset();

	inline const auto& getHandles() const { return devHandles; }
	inline int getDevCount() const { return devCount; }
};