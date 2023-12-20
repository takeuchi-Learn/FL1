#include "PadImu.h"
#include <Input/Input.h>

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

bool PadImu::checkInputAccept() const
{
	constexpr auto useKey = DIK_SPACE;
	constexpr auto useXInputButton = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B;
	constexpr auto useJSLMask = JSMASK_E | JSMASK_S;

	if (Input::ins()->triggerKey(useKey)) { return true; }

	if (Input::ins()->triggerPadButton(useXInputButton))
	{
		return true;
	}

	if (PadImu::ins()->getDevCount() > 0)
	{
		const int preState = PadImu::ins()->getPreStates()[0].buttons;
		const int state = PadImu::ins()->getStates()[0].buttons;

		const bool pre = PadImu::hitButtons(preState, useJSLMask);
		const bool current = PadImu::hitButtons(state, useJSLMask);

		if (!pre && current) { return true; }
	}

	return false;
}

bool PadImu::checkInputDPAD(size_t arrayNum, int direction)
{
	return hitButtons(joyShockStates[arrayNum].buttons, direction);
}

bool PadImu::checkTriggerInputDPAD(size_t arrayNum,
								   int direction)
{
	return hitButtons(joyShockStates[arrayNum].buttons, direction) &&
		!hitButtons(joyShockPreStates[arrayNum].buttons, direction);
}