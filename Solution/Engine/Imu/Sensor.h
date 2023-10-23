#pragma once
#include <array>
#include <vector>
#include "ini.h"
#include "serial.h"

class Sensor
{
public:
	static Sensor* create();
	~Sensor();
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

private:
	static const int accelCount = 3;
	static const int gyroCount = 3;
	static const int sensorCount = accelCount + gyroCount;

	Sensor(const char* serialDevice);
	int updateSensor();
	const int maxRecordCount = 1920;
	bool isRecording = true;
	bool isFlashing = false;
	Serial* serial;
	std::array<float, sensorCount> record;
	int fps = 0;
	int dps = 0;
};
