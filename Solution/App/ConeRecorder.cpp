#include "ConeRecorder.h"

bool ConeRecorder::registration(const uint16_t stageNum, const uint16_t coneCount)
{
	try
	{
		auto& record = records.at(stageNum);
		if (coneCount > record) { record = coneCount; }
	} catch (...)
	{
		return true;
	}

	return false;
}