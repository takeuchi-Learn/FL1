#pragma once
#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#define M_PI 3.14159265358979323846f

class KalmanFilter
{
public:

	float Uniform();

	float rand_normal(float mu, float sigma);

	float rand_normal2(float mu, float sigma);

	void Update();

private:
	float mu_v			 = 0.0f;
	float sig_v			 = 2.0f;
	float mu_w			 = 0.0f;
	float sig_w			 = 1.0f;
	float t				 = 0.0f;
	float dt			 = 0.01f;
	float u				 = 0.0f;
	float w				 = 0.0f;
	float v				 = 0.0f;
	float x				 = 0.0f;
	float dx			 = 0.0f;
	float xold			 = 0.0f;
	float y				 = 0.0f;
	float kg			 = 0.0f;
	float S				 = 0.0f;
	float e				 = 0.0f;
	float x_priori		 = 0.0f;
	float P_priori		 = 0.0f;
	float x_hat			 = 0.0f;
	float P_hat			 = 1.0f;
	float state_error	 = 0.0f;
};