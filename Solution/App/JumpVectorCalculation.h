#pragma once

// ジャンプ時のベクトルを計算する関数
// プレイヤーのジャンプ処理をカラーコーンでも使うため個別にクラス化
class JumpVectorCalculation
{
public:
	// 時間などもこちらで管理できるようにしようと思ったが、
	// プレイヤーの書き換えが発生しバグの原因になったら困るため、修正する余裕ができるまでstatic関数

	/// @brief 自由落下(投げ上げ)の速度を計算します。
	/// @param startVel 初速度
	/// @param gravAcc 加速度
	/// @param t 時間
	/// @return
	static float calcFallVector
	(
		const float startVel,
		const float gravAcc,
		const int t
	)
	{
		return startVel + -gravAcc * static_cast<float>(t);
	}

};

