#pragma once
#include <windows.h>

class Serial
{
public:
	static Serial* create(const char* dev);
	~Serial();
	int read(char* buf, unsigned int bufSize);
private:
	Serial(HANDLE fd);
	HANDLE fd;
};
