#include "CircularGauge.hlsli"

VSOutput main(float4 pos : POSITION,
			  float scale : TEXCOORD,
			  float4 color : COLOR,
			  float holeSize : HOLE_SIZE,
			  float gaugeRaito : GAUGE_RAITO)
{
	VSOutput output; // ピクセルシェーダーに渡す値
	output.pos = pos;
	output.scale = scale;
	output.color = color;
	output.holeSize = holeSize;
	output.gaugeRaito = gaugeRaito;
	return output;
}