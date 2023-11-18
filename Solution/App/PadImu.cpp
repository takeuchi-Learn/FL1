#include "PadImu.h"
#include <JoyShockLibrary.h>

PadImu::PadImu()
{
	init();
}

void PadImu::init()
{
	devCount = JslConnectDevices();
	if (devCount > 0)
	{
		devHandles.resize(devCount, -1);
		JslGetConnectedDeviceHandles(devHandles.data(), devCount);
	}
}

void PadImu::reset()
{
	JslDisconnectAndDisposeAll();

	init();
}
