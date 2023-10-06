#include "BaseGameObjectHavingHp.h"

using namespace DirectX;

bool BaseGameObjectHavingHp::damage(uint16_t damegeNum, bool killFlag)
{
	for (auto& i : preDamageProc)
	{
		i();
	}

	if (damegeNum >= this->hp)
	{
		this->hp = 0u;
		if (killFlag) { kill(); }
		return true;
	}

	this->hp -= damegeNum;
	return false;
}