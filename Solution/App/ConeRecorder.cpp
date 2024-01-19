#include "ConeRecorder.h"

bool ConeRecorder::registration(const uint16_t stageNum, const uint16_t coneCount)
{
	try
	{
		auto& record = records.at(stageNum);
		if (coneCount > record) { record = coneCount; }
	} catch (...)
	{
		records.emplace(stageNum, coneCount);
		return true;
	}

	return false;
}