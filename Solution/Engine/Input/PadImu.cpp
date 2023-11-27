#include "PadImu.h"

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
		joyShockPreStates.resize(devCount, JOY_SHOCK_STATE{});
		joyShockStates.resize(devCount, JOY_SHOCK_STATE{});
		JslGetConnectedDeviceHandles(devHandles.data(), devCount);
	}
}

void PadImu::reset()
{
	JslDisconnectAndDisposeAll();

	init();
}

void PadImu::update()
{
	for (int i = 0; i < devCount; ++i)
	{
		joyShockPreStates[i] = joyShockStates[i];
		joyShockStates[i] = JslGetSimpleState(devHandles[i]);
	}
}