#include <Windows.h>
#include <memory>

#include "System.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	auto sys = std::make_unique<System>();

	sys->update();

	return 0;
}