#include "ConeRecorder.h"

void ConeRecorder::registration(const unsigned short stageNum, const unsigned short coneCount)
{
	try
	{
		auto& record = records.at(stageNum);
		if (coneCount > record) { record = coneCount; }
	} catch (...)
	{
		return;
	}
}
