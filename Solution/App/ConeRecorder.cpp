#include "ConeRecorder.h"

void ConeRecorder::registration(const unsigned short stageNum, const unsigned short coneCount)
{
	if (coneCount > records.at(stageNum))records[stageNum] = coneCount;
}
