#include "PbrObj.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー

#define PI (3.141592653589793f)
#define RecPI (0.31830988618379067153776752674503f)

#define BRDFEnergyFactMax (0.6622516556291390728476821192053f)

/// @brief Schlickによる近似（ [f0 ~ f90] を、(1-cos)^5で線形補間する。）
/// @param f0 光が0°（垂直）に入射したときの反射率
/// @param f90 光が90°（平行）に入射したときの反射率
/// @param 2つのベクトルが成す角のコサイン（内積値）
float SchlickFresnel(float f0, float f90, float cosine)
{
	const float m = saturate(1.f - cosine);
	const float mm = m * m;
	const float m5 = mm * mm * m;
	return lerp(f0, f90, m5);
}

float DistributionGGX(float alpha, float normDotHalf)
{
	const float alpha2 = alpha * alpha;
	const float t = normDotHalf * normDotHalf * (alpha2 - 1.f) + 1.f;
	return alpha2 / (PI * t * t);
}

float3 SchlickFresnel3(float3 f0, float3 f90, float cosine)
{
	float m = saturate(1 - cosine);
	float m2 = m * m;
	float m5 = m2 * m2 * m;
	return lerp(f0, f90, m5);
}

float3 DisneyFresnel(float lightDotHalf, float3 baseColor, float metallic)
{
	//F項(フルネル : Fresnel)
	//輝度
	float liminance = 0.3f * baseColor.r + 0.6f * baseColor.g + 0.1f * baseColor.b;
	//色合い
	float3 tintColor = baseColor / liminance;
	//非金属の鏡面反射色の計算
	float3 nonMetarlColor = m_specular * 0.08f * tintColor;
	//metalnessによる色補間 金属の場合はベースカラー
	float3 specularColor = lerp(nonMetarlColor, baseColor, metallic);

	//NdptHの割合でSchlickFresnel補間
	return SchlickFresnel3(specularColor, float3(1, 1, 1), lightDotHalf);
}

float GeometricSmith(float cosine, float roughness)
{
	float k = (roughness + 1.f);
	k *= k / 8.f;

	return cosine / (cosine * (1.f - k) + k);
}

float3 CookTorranceSpecular(float normDotLight,
							float normDotCamera,
							float normDotHalf,
							float lightDotHalf,
							float roughness,
							float metallic,
							float3 baseColor)
{
	// D項
	float Ds = DistributionGGX(roughness * roughness, normDotHalf);
	// F項(分布 : Distribution)
	float3 Fs = DisneyFresnel(lightDotHalf, baseColor, metallic);
	// G項(幾何減数 : Geometry attenuation)
	float Gs = GeometricSmith(normDotLight, roughness) * GeometricSmith(normDotCamera, roughness);
	// m項(分母)
	float m = 4.f * normDotLight * normDotCamera;

	// 合成して鏡面反射の色を返す
	return Ds * Fs * Gs / m;
}

static float3 normal_BRDF = float3(0, 0, 0);
static float3 cameraDir_BRDF = float3(0, 0, 0);

// @brief 双方向反射分布
float3 BRDF(float3 lightDir)
{
	const float normDotLight = dot(normal_BRDF, lightDir);
	const float normDotCamera = dot(normal_BRDF, cameraDir_BRDF);
	
	// 直角以上なら真っ黒
	if (normDotLight < 0.f ||
		normDotCamera < 0.f)
	{
		return float3(0.f, 0.f, 0.f);
	}
	
	// ハーフベクトル: ライト方向とカメラ方向の中間
	const float3 halfLightCam = normalize(lightDir + cameraDir_BRDF);
	
	// 法線とハーフベクトルの内積
	const float normDotHalf = dot(normal_BRDF, halfLightCam);
	// ライトとハーフベクトルの内積
	const float lightDotHalf = dot(lightDir, halfLightCam);
	
	const float metallic = m_ambient.x;
	const float roughness = m_diffuse.x;
	
	// 入射角が90°のときの拡散反射率
	const float f90Deg = (0.5f * roughness) + 2.f * lightDotHalf * lightDotHalf * roughness;
	
	// 入っていくときの拡散反射率
	const float fIn = SchlickFresnel(1.f, f90Deg, normDotLight);
	// 出ていくときの拡散反射率
	const float fOut = SchlickFresnel(1.f, f90Deg, normDotCamera);
	
	const float energyFact = lerp(1.f, BRDFEnergyFactMax, roughness);
	// 入ってから出るまでの拡散反射率
	const float fd = fIn * fOut * energyFact;
	
	// RecPI: 1 / PI
	const float3 diffuseColor = RecPI * fd * color.rgb * (1.f - metallic);
	
	const float3 specularColor = CookTorranceSpecular(normDotLight,
													  normDotCamera,
													  normDotHalf,
													  lightDotHalf,
													  roughness,
													  metallic,
													  color.rgb);
	
	return diffuseColor + specularColor;
}

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
	const float4 texcolor = float4(tex.Sample(smp, input.uv * texTilling + shiftUv));

	ScreenDoor(input.svpos.xy, texcolor.a * color.a * m_alpha);
	//ScreenDoor2_xy(input.svpos.xy, texcolor.a * color.a * m_alpha, 8.f);
	
	cameraDir_BRDF = normalize(cameraPos - input.worldPos.xyz); // 頂点->視点ベクトル
	
	normal_BRDF = input.normal; // 頂点法線
	
	float4 shadeColor = float4(ambientColor, 1.f);
	
	for (uint d = 0; d < activeDirLightCount; ++d)
	{
		shadeColor.rgb += BRDF(normalize(dirLights[d].dir2Light)) *
						  dirLights[d].color;
	}
	
	for (uint p = 0; p < activePointLightCount; ++p)
	{
		float3 dir2Light = pointLights[p].pos - input.worldPos.xyz;
		const float l = length(dir2Light);
		dir2Light = normalize(dir2Light);
		
		const float atten = 1.f / (pointLights[p].atten.x + pointLights[p].atten.y * l + pointLights[p].atten.z * l * l);
		
		shadeColor.rgb += atten *
							BRDF(dir2Light) *
							pointLights[p].color.rgb;
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
		
		shadeColor.rgb += atten *
							BRDF(dir2Light) *
							spotLights[p].color.rgb;
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
	output.target0 = float4(shadeColor.rgb * texcolor.rgb, 1.f);
	output.target1 = float4(texcolor.rgb, 1.f);

	return output;
}
