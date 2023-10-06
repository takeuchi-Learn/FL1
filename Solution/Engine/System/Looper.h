#pragma once
#include "PostEffect.h"
#include <memory>

class Looper
{
private:
	bool exitFlag = false;

	Looper(const Looper& looper) = delete;
	Looper& operator=(const Looper& looper) = delete;

	Looper();

	void loopUpdate();
	void loopDraw();

	void pushImGuiCol();
	void popImGuiCol();

public:
	~Looper();

	static inline Looper* getInstance()
	{
		static std::unique_ptr<Looper> lp(new Looper());
		return lp.get();
	}
	static inline Looper* ins() { return getInstance(); }

	inline void exitGame() { exitFlag = true; }

	// @return 異常の有無(falseで正常)
	bool loop();
};
