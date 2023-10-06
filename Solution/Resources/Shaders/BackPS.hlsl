#include "Back.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

PSOutput main(VSOutput input)
{
	float3 ambient = m_ambient;
	float4 shadeColor = float4(ambientColor * ambient, m_alpha);
		
	for (uint c = 0; c < activeCircleShadowCount; ++c)
	{
		// オブジェクト表面からキャスターへのベクトル
		float3 casterv = circleShadows[c].casterPos - input.worldPos.xyz;
		// 光線方向での距離
		float d = dot(casterv, circleShadows[c].invLightDirNormal);

		// 距離減衰係数
		float atten = saturate(1.f / (circleShadows[c].atten.x + circleShadows[c].atten.y * d + circleShadows[c].atten.z * d * d));
		// 距離がマイナスなら0にする
		atten *= step(0, d);

		// ライトの座標
		float3 lightpos = circleShadows[c].casterPos + circleShadows[c].invLightDirNormal * circleShadows[c].caster2LightDistance;
		//  オブジェクト表面からライトへのベクトル（単位ベクトル）
		float3 lightv = normalize(lightpos - input.worldPos.xyz);
		// 角度減衰
		float cos = dot(lightv, circleShadows[c].invLightDirNormal);
		// 減衰開始角度から、減衰終了角度にかけて減衰
		// 減衰開始角度の内側は1倍 減衰終了角度の外側は0倍の輝度
		float angleatten = smoothstep(circleShadows[c].factorAngleCos.y, circleShadows[c].factorAngleCos.x, cos);
		// 角度減衰を乗算
		atten *= angleatten;

		// 全て減算する
		shadeColor.rgb -= atten;
	}

	
	PSOutput output;
	float4 texcolor = float4(tex.Sample(smp, input.uv * texTilling + shiftUv));
	output.target0 = shadeColor * texcolor * color;
	output.target1 = texcolor;

	return output;
}
