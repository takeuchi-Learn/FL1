#include "CircularGauge.hlsli"

#define PI (3.141592653589793f)
#define PI2 (6.283185307179586476925286766559f)
#define PI_DIV2 (1.5707963267948966192313216916398f)

// ディザリング抜き
void ScreenDoor(float2 screenPos, float alpha)
{
	// 0 ~ 1
	static const float Bayer[4][4] =
	{
		{ 0.0000f, 0.5000f, 0.1250f, 0.6250f },
		{ 0.7500f, 0.2500f, 0.8750f, 0.3750f },
		{ 0.1875f, 0.6875f, 0.0625f, 0.5625f },
		{ 0.9375f, 0.4375f, 0.8125f, 0.3125f }
	};
	
	// 0 ~ 1
	float ditherLevel = 1.f - saturate(alpha);
	int ditherUvX = int(screenPos.x) % 4;
	int ditherUvY = int(screenPos.y) % 4;
	float doorNum = Bayer[ditherUvY][ditherUvX];
	clip(doorNum - ditherLevel);
}

float circle(float2 position, float size, float feather, float distance2Pos)
{
	return smoothstep(size, size + feather, distance2Pos);
}

PSOutput main(GSOutput input)
{
	static float val = 0.f;
	static float2 pos = float2(0.f, 0.f);
	static float distance2Pos = 0.f;
	static float angle01 = 0.f;
	
	pos = input.uv - float2(0.5f, 0.5f);
	distance2Pos = dot(pos, pos) * 4.f;
	val = 1.f - circle(pos, 1.f, 0.005f, distance2Pos);
	val *= circle(pos, input.holeSize, 0.005f, distance2Pos);
	
	// 角度[Rad]を取得
	angle01 = atan2(pos.y, pos.x);
	// 範囲を0 ~ 1にする
	angle01 = (angle01 + PI) / PI2;
	// 始点を上にする（左→上）
	angle01 = frac(angle01 + 0.75f);
	// 値を反映させる
	val *= step(angle01, input.gaugeRaito);
	
	ScreenDoor(input.svpos.xy, val * input.color.a);
	
	PSOutput output;
	output.target0 = float4(val * input.color.rgb, 1.f);
	output.target1 = float4(1.f - input.color.rgb, 1.f);

	return output;
}