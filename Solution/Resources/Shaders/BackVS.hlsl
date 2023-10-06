#include "Back.hlsli"

VSOutput main(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	float4 wnormal = normalize(mul(world, float4(normal, 0)));
	float4 wpos = mul(world, pos);
	float4 viewPos = mul(view, wpos);
	
	VSOutput output;
	output.svpos = mul(proj, viewPos);
	output.worldPos = wpos;
	output.viewPos = viewPos;
	output.normal = wnormal.xyz;
	output.uv = uv;
	return output;
}
