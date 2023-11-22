#pragma once
#include <array>
#include <memory>
#include "ini.h"
#include "serial.h"

class Sensor
{
public:
	Sensor();

	~Sensor();

	// 更新
	bool update();

	// 加速度X(傾き)
	float GetAccelX()
	{
		return record[0] / 16384.0f;
	}
	// 加速度Y
	float GetAccelY()
	{
		return record[1] / 16384.0f;
	}
	// 加速度Z(ジャンプ)
	float GetAccelZ()
	{
		return record[2] / 16384.0f;
	}
	// 角速度X
	float GetGyroX()
	{
		return record[3] / 131.0f;
	}
	// 角速度Y
	float GetGyroY()
	{
		return record[4] / 131.0f;
	}
	// 角速度XZ
	float GetGyroZ()
	{
		return record[5] / 131.0f;
	}

private:
	uint32_t updateSensor();

	static constexpr uint32_t accelCount = 3u;
	static constexpr uint32_t gyroCount = 3u;
	static constexpr uint32_t sensorCount = accelCount + gyroCount;

	std::unique_ptr<Serial> serial;
	std::array<float, sensorCount> record{};
};
