#include "DistanceDither.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

// ディザリング抜き
void ScreenDoor(float2 screenPos, float alpha)
{
	// 0 ~ 15
	//static const int Bayer[4][4] =
	//{
	//	{ 0, 8, 2, 10 },
	//	{ 12, 4, 14, 6 },
	//	{ 3, 11, 1, 9 },
	//	{ 15, 7, 13, 5 }
	//};
	
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

/// @param value 透明度とclip線がvalue段階になる
void ScreenDoor2_x(float2 screenPos, float alpha, float value = 4.f)
{
	const float doorNum = (1.f - alpha) * value;
	const float posVal = fmod(screenPos.x, value);
	clip(posVal - doorNum);
}

void ScreenDoor2_y(float2 screenPos, float alpha, float value = 4.f)
{
	const float doorNum = (1.f - alpha) * value;
	const float posVal = fmod(screenPos.y, value);
	clip(posVal - doorNum);
}

void ScreenDoor2_xy(float2 screenPos, float alpha, float value = 4.f)
{
	const float doorNum = (1.f - alpha) * value;
	const float posVal = min(fmod(screenPos.x, value), fmod(screenPos.y, value));
	clip(posVal - doorNum);
}

PSOutput main(VSOutput input)
{
	float4 texcolor = float4(tex.Sample(smp, input.uv * texTilling + shiftUv));

	float clipAlpha = texcolor.a * color.a * m_alpha;
	
	clipAlpha *= clamp(max(0.f, length(input.viewPos) - 22.f) / 4.f,
					   0.2f,
					   1.f);

	ScreenDoor(input.svpos.xy, clipAlpha);
	
	const float shininess = 4.f; // 光沢
	float3 eyeDir = normalize(cameraPos - input.worldPos.xyz); // 頂点->視点ベクトル
	
	float3 ambient = m_ambient;
	float4 shadeColor = float4(ambientColor * ambient, m_alpha);
	
	for (uint d = 0; d < activeDirLightCount; ++d)
	{
		// ライトへ向かうベクトルと法線の内積
		float3 dir2LightDotNormal = dot(dirLights[d].dir2Light, input.normal);
		float3 reflect = normalize(-dirLights[d].dir2Light + 2 * dir2LightDotNormal * input.normal);
		float3 diffuse = dir2LightDotNormal * m_diffuse;
		float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular;
		
		shadeColor.rgb += (diffuse + specular) * dirLights[d].color;
	}
	
	for (uint p = 0; p < activePointLightCount; ++p)
	{
		float3 dir2Light = pointLights[p].pos - input.worldPos.xyz;
		float l = length(dir2Light);
		dir2Light = normalize(dir2Light);
		
		float atten = 1.f / (pointLights[p].atten.x + pointLights[p].atten.y * l + pointLights[p].atten.z * l * l);
		
		// ライトへ向かうベクトルと法線の内積
		float3 dir2LightDotNormal = dot(dir2Light, input.normal);

		float3 reflect = normalize(-dir2Light + 2 * dir2LightDotNormal * input.normal); // 反射光
		float3 diffuse = dir2LightDotNormal * m_diffuse;
		float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular; // 鏡面反射光

		shadeColor.rgb += atten * (diffuse + specular) * pointLights[p].color.rgb;
	}
	
	for (uint s = 0; s < activeSpotLightCount; ++s)
	{
		// ライトへの方向ベクトル
		float3 dir2Light = spotLights[s].pos - input.worldPos.xyz;
		float d = length(dir2Light);
		dir2Light = normalize(dir2Light);

		// 距離減衰係数
		float atten = saturate(1.f / (spotLights[s].atten.x + spotLights[s].atten.y * d + spotLights[s].atten.z * d * d));

		// 角度減衰
		float cos = dot(dir2Light, spotLights[s].invLightDirNormal);
		// 減衰開始角度から、減衰終了角度にかけて減衰
		// 減衰開始角度の内側は1倍 減衰終了角度の外側は0倍の輝度
		float angleatten = smoothstep(spotLights[s].factorAngleCos.y, spotLights[s].factorAngleCos.x, cos);
		// 角度減衰を乗算
		atten *= angleatten;

		// ライトに向かうベクトルと法線の内積
		float3 dir2LightDotNormal = dot(dir2Light, input.normal);
		// 反射光ベクトル
		float3 reflect = normalize(-dir2Light + 2 * dir2LightDotNormal * input.normal); // 反射光
		// 拡散反射光
		float3 diffuse = dir2LightDotNormal * m_diffuse;
		// 鏡面反射光
		float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular; // 鏡面反射光

		// 全て加算する
		shadeColor.rgb += atten * (diffuse + specular) * spotLights[s].color;
	}
	
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
	output.target0 = float4(shadeColor.rgb * texcolor.rgb * color.rgb, 1.f);
	output.target1 = float4(texcolor.rgb, 1);

	return output;
}
