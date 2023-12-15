#include <windows.h>
#include "Sensor.h"

Sensor::Sensor()
{
	serial.reset(Serial::create("COM3"));
}

bool Sensor::update()
{
	return this->updateSensor() > 0;
}

int Sensor::updateSensor()
{
	if (!this->serial) { return 0; }

	static char buf[1024]{};
	static int contentSize = 0;
	const int receivedSize = this->serial->read(buf + contentSize, sizeof(buf) - contentSize);
	if (receivedSize == -1) { return 0; }
	contentSize += receivedSize;

	char* p = buf;
	constexpr char header[] = { 'D', 'A', 'T', 'F' };
	constexpr int packetSize = sizeof(header) + sizeof(int16_t) * Sensor::sensorCount;

	int dataCount = 0;

	for (; p < buf + contentSize - packetSize; )
	{
		if (memcmp(p, header, sizeof(header)) != 0)
		{
			p++;
			continue;
		}
		++dataCount;
		p += sizeof(header);

		for (int i = 0; i < Sensor::sensorCount; i++, p += sizeof(int16_t))
		{
			constexpr float accelSensitivity_16g = 16384.f;
			constexpr float gyroSensitivity_2kdps = 131.f;
			record[i] = *((const int16_t*)p) / (i < Sensor::accelCount ? accelSensitivity_16g : gyroSensitivity_2kdps);
		}
	}

	if (receivedSize == 0) { isButton = true; } else { isButton = false; }

	const int tailSize = (int)(buf + contentSize - p);
	memmove(buf, p, tailSize);
	contentSize = tailSize;
	return dataCount;
}