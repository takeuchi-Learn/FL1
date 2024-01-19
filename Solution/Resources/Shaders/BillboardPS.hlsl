#include "Billboard.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

PSOutput main(GSOutput input)
{
	PSOutput output;
	output.target1 = tex.Sample(smp, input.uv);
	output.target0 = output.target1 * input.color;

	return output;
}