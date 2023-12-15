#pragma once
#include <array>
#include <memory>
#include "serial.h"

class Sensor
{
private:
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
	float GetAccelX()
	{
		return record[0];
	}
	// 加速度Y
	float GetAccelY()
	{
		return record[1];
	}
	// 加速度Z(ジャンプ)
	float GetAccelZ()
	{
		return record[2];
	}
	// 角速度X
	float GetGyroX()
	{
		return record[3];
	}
	// 角速度Y
	float GetGyroY()
	{
		return record[4];
	}
	// 角速度XZ
	float GetGyroZ()
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
