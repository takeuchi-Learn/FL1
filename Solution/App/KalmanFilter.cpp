#include "KalmanFilter.h"

float KalmanFilter::Uniform()
{
	return ((float)rand() + 1.0f) / ((float)RAND_MAX + 2.0f);
}

float KalmanFilter::rand_normal(float mu, float sigma)
{

	float z = sqrtf(-2.0f * logf(Uniform())) * sinf(2.0f * M_PI * Uniform());

	return mu + sigma * z;
}

float KalmanFilter::rand_normal2(float mu, float sigma)
{

	float z = sqrtf(-2.0f * logf(Uniform())) * sinf(2.0f * M_PI * Uniform());

	return mu + sigma * z;
}

void KalmanFilter::Update()
{
	u = cosf(3.0f * t); //入力

	v = rand_normal(mu_v, sig_v); //システム雑音
	w = rand_normal2(mu_w, sig_w); //観測雑音

	x = xold + u + v; //状態方程式
	y = x + w; //状態方程式 yが観測できる信号でこの値からxをx_hatとして推定する

	//----------------------カルマンフィルタ-----------------------------//
	//予測ステップ//
	x_priori = x_hat + u; //事前状態予測
	P_priori = P_hat + sig_v; //事前誤差共分散予測

	//更新ステップ//
	e = y - x_priori; //観測残差　観測雑音平均が分かっている場合はyをy-mu_wにすればよい。
	S = sig_w + P_priori; //観測残差の共分散
	kg = P_priori / S; //カルマンゲイン更新
	x_hat = x_priori + kg * e; //状態推定
	P_hat = (1.0f - kg) * P_priori; //誤差共分散
	//--------------------------------------------------------------//
	state_error = x - x_hat; //状態推定誤差
	xold = x; //次ステップで用いる状態推定値
	t += dt; //時間の更新

	//printf("%lf %lf %lf %lf %lf\n", t, x, x_hat, y, state_error);
}
