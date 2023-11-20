#pragma once
#include <array>
#include <vector>
#include "ini.h"
#include "serial.h"

class Sensor
{
private:
	Sensor(const char* serialDevice);

	~Sensor();
public:
	// 生成
	static Sensor* create();
	// 更新
	bool update();
	// 消去
	void erase();

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
	int updateSensor();

	static const int accelCount = 3;
	static const int gyroCount = 3;
	static const int sensorCount = accelCount + gyroCount;

	Serial* serial;
	std::array<float, sensorCount> record;
};
