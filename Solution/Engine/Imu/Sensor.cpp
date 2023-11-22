#include <windows.h>
#include "Sensor.h"

Sensor::Sensor()
{
	serial.reset(Serial::create("COM3"));
}

Sensor::~Sensor()
{}

bool Sensor::update()
{
	return this->updateSensor() > 0;
}

uint32_t Sensor::updateSensor()
{
	if (!this->serial) { return 0; }

	static char buf[1024]{};
	static uint32_t contentSize = 0;
	const int receivedSize = this->serial->read(buf + contentSize, sizeof(buf) - contentSize);
	if (receivedSize == -1) { return 0; }
	contentSize += static_cast<uint32_t>(receivedSize);

	char* p = buf;
	constexpr char header[] = { 'D', 'A', 'T', 'F' };
	constexpr uint32_t packetSize = sizeof(header) + sizeof(int16_t) * Sensor::sensorCount;

	uint32_t dataCount = 0;

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
			constexpr float accelSensitivity_16g = 2048.f;
			constexpr float gyroSensitivity_2kdps = 16.4f;
			record[i] = *((const int16_t*)p)/* / (i < Sensor::accelCount ? accelSensitivity_16g : gyroSensitivity_2kdps)*/;
		}
	}

	const int tailSize = (int)(buf + contentSize - p);
	memmove(buf, p, tailSize);
	contentSize = tailSize;
	return dataCount;
}