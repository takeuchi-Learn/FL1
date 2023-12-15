#include "ConeRecorder.h"

void ConeRecorder::registration(const unsigned short stageNum, const unsigned short coneCount)
{
	if (coneCount > records[stageNum])records[stageNum] = coneCount;
}
