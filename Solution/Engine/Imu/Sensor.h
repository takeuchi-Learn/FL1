#pragma once
#include <array>
#include <memory>
#include "serial.h"

class Sensor
{
private:
	// シングルトン
	Sensor(const Sensor&) = delete;
	Sensor& operator=(const Sensor&) = delete;
	Sensor();

public:
	inline static Sensor* ins()
	{
		static Sensor sensor{};
		return &sensor;
	}

	// 更新
	bool update();

	// 加速度X(傾き)
	inline float GetAccelX() const
	{
		return record[0];
	}
	// 加速度Y
	inline float GetAccelY() const
	{
		return record[1];
	}
	// 加速度Z(ジャンプ)
	inline float GetAccelZ() const
	{
		return record[2];
	}
	// 角速度X
	inline float GetGyroX() const
	{
		return record[3];
	}
	// 角速度Y
	inline float GetGyroY() const
	{
		return record[4];
	}
	// 角速度XZ
	inline float GetGyroZ() const
	{
		return record[5];
	}

	inline bool GetButton() const { return buttonState; }
	inline bool GetPreButton() const { return buttonStatePre; }
	inline bool CheckButton() const { return !buttonStatePre && buttonState; }

private:
	int updateSensor();

	static constexpr int accelCount = 3;
	static constexpr int gyroCount = 3;
	static constexpr int sensorCount = accelCount + gyroCount;

	bool buttonState = false;
	bool buttonStatePre = false;

	std::unique_ptr<Serial> serial;
	std::array<float, sensorCount> record{};
};
