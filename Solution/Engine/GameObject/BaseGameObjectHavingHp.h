#pragma once
#include "AbstractGameObj.h"

#include <forward_list>
#include <functional>

/// @brief ゲームオブジェクト基底クラス
class BaseGameObjectHavingHp : public AbstractGameObj
{
protected:
	uint16_t hp = 1ui16;

private:
	/// @brief ダメージを受けた時に行う処理
	std::forward_list<std::function<void()>> preDamageProc;

public:
	using AbstractGameObj::AbstractGameObj;

#pragma region アクセッサ

	inline void setHp(uint16_t _hp) { this->hp = _hp; }
	inline uint16_t getHp() const { return hp; }

#pragma endregion アクセッサ

	/// @brief ダメージを受けた時に行う処理を追加
	/// @return 追加した要素
	inline auto addPreDamageProc(const std::function<void()>& proc) { return preDamageProc.emplace_front(proc); }

	/// @brief ダメージを与える
	/// @param damageNum 与えるダメージ数
	/// @param killFlag hpが0になったらkillするかどうか(trueでkillする)
	/// @return 倒したかどうか(倒したらtrue)
	bool damage(uint16_t damegeNum, bool killFlag = true);
};
