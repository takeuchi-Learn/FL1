#include <windows.h>
#include "Sensor.h"

//int WINAPI WinMain(_In_ HINSTANCE , _In_opt_ HINSTANCE , _In_ LPSTR lpCmdLine, _In_ int )
//{
//	const char platform[] = "Windows";
//	Sensor* sensor = Sensor::create(platform);
//	if (!sensor) return 1;
//	while (sensor->update());
//	delete sensor;
//	return 0;
//}

Sensor* Sensor::create()
{
	std::string serialDevice;
	Ini ini = Ini("viewer.ini");
	serialDevice = ini.getValue("Windows", "serialDevice");
	return new Sensor(serialDevice.c_str());
}

Sensor::Sensor(const char* serialDevice)
{
	this->serial = Serial::create(serialDevice);
}

Sensor::~Sensor()
{
	if (this->serial) delete this->serial;
}

bool Sensor::update()
{
	const int dataCount = this->updateSensor();
	return true;
}

int Sensor::updateSensor()
{
	int dataCount = 0;
	if (!this->serial) return dataCount;

	static char buf[1024];
	static int contentSize = 0;
	const int receivedSize = this->serial->read(buf + contentSize, sizeof(buf) - contentSize);
	if (receivedSize == -1) return dataCount;
	contentSize += receivedSize;

	char* p = buf;
	const char header[] = { 'D', 'A', 'T', 'F' };
	const int packetSize = sizeof(header) + sizeof(int16_t) * Sensor::sensorCount;

	for ( ; p < buf + contentSize - packetSize; )
	{
		if (memcmp(p, header, sizeof(header)) != 0)
		{
			p++;
			continue;
		}
		dataCount++;
		p += sizeof(header);

		for (int i = 0; i < Sensor::sensorCount; i++, p += sizeof(int16_t))
		{
			const float accelSensitivity_16g = 2048;
			const float gyroSensitivity_2kdps = 16.4f;
			record[i] = *((const int16_t*)p) / (i < Sensor::accelCount ? accelSensitivity_16g : gyroSensitivity_2kdps);
		}

		//// 記録中
		//if (this->isRecording)
		//{
		//	this->records.push_back(record);
		//	// 表示が最大まで行ったら
		//	if (this->records.size() > this->maxRecordCount)
		//	{
		//		// 古い表示を消す
		//		this->records.erase(this->records.begin());
		//	}
		//}
	}

	const int tailSize = (int)(buf + contentSize - p);
	memmove(buf, p, tailSize);
	contentSize = tailSize;
	return dataCount;
}
