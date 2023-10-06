cbuffer cbuff0 : register(b0)
{
	float4 color; // 色(RGBA)
	matrix view;
	matrix proj;
	matrix world; // ワールド行列
	float3 cameraPos; // カメラ位置(ワールド座標)
};

cbuffer cbuff1 : register(b1)
{
	float3 m_ambient : packoffset(c0); // アンビエント係数
	float3 m_diffuse : packoffset(c1); // ディフューズ係数
	float3 m_specular : packoffset(c2); // ディフューズ係数
	float m_alpha : packoffset(c2.w); // アルファ
	float2 texTilling : packoffset(c3);
	float2 shiftUv : packoffset(c4);
}

// 平行光源の最大数
// CPP側と合わせる
#define DirLightCountMax uint(8)

struct DirectionalLight
{
	float3 dir2Light; // ライトへの方向の単位ベクトル
	float3 color; // ライトの色(RGB)
};

// 点光源の最大数
// CPP側と合わせる
#define PointLightCountMax uint(3)

struct PointLight
{
	float3 pos; // ライト座標
	float3 color; // ライトの色(RGB)
	float3 atten; // ライト距離減衰係数
};

// スポットライトの最大数
// CPP側と合わせる
#define SpotLightCountMax uint(3)

struct SpotLight
{
	float3 invLightDirNormal; // ライトの光線方向の逆ベクトル（単位ベクトル）
	float3 pos; // ライト座標
	float3 color; // ライトの色(RGB)
	float3 atten; // ライト距離減衰係数
	float2 factorAngleCos; // ライト減衰角度のコサイン
};

// 丸影の最大数
// CPP側と合わせる
#define CircleShadowCountMax uint(32)

struct CircleShadow
{
	float3 invLightDirNormal;
	float3 casterPos;
	float caster2LightDistance;
	float3 atten;
	float2 factorAngleCos;
};

cbuffer cbuff2 : register(b2)
{
	float3 ambientColor;
	uint activeDirLightCount;
	uint activePointLightCount;
	uint activeSpotLightCount;
	uint activeCircleShadowCount;
	DirectionalLight dirLights[DirLightCountMax];
	PointLight pointLights[PointLightCountMax];
	SpotLight spotLights[SpotLightCountMax];
	CircleShadow circleShadows[CircleShadowCountMax];
};

// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VSOutput
{
	float4 svpos : SV_POSITION; // システム用頂点座標
	float4 viewPos : COLOR;
	float4 worldPos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD; // uv値
};

// レンダーターゲットの数は2つ
// PostEffectクラスのRenderTargetNumは此処の個数(2つ)に合わせる
// 配列にすればいけるのでは?
struct PSOutput
{
	float4 target0 : SV_TARGET0;
	float4 target1 : SV_TARGET1;
};