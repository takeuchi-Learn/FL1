#pragma once

#include <vector>
#include <JoyShockLibrary.h>

class PadImu
{
private:
	PadImu(const PadImu&) = delete;
	PadImu& operator=(const PadImu&) = delete;
	PadImu();

private:
	std::vector<int> devHandles{};
	std::vector<JOY_SHOCK_STATE> joyShockPreStates{};
	std::vector<JOY_SHOCK_STATE> joyShockStates{};
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

	void update();

	inline const auto& getHandles() const { return devHandles; }
	inline int getDevCount() const { return devCount; }

	inline const auto& getStates() const { return joyShockStates; }
	inline const auto& getPreStates() const { return joyShockPreStates; }

	/// @param arrayNum getHandlesの配列の添え字
	inline int getContollerType(size_t arrayNum) const
	{
		return JslGetControllerType(devHandles[arrayNum]);
	}

	static inline constexpr bool hitButtons(int stateButtons, int JSMASK)
	{
		return static_cast<bool>(stateButtons & JSMASK);
	}
};