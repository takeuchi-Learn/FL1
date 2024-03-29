cbuffer cbuff0 : register(b0)
{
	matrix mat; // ビュープロジェクション行列
	matrix matBillboard; // ビルボード行列
};

// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct VSOutput
{
	float4 pos : POSITION; // 頂点座標
	float scale : TEXCOORD; // スケール
	float4 color : COLOR;
	float holeSize : HOLE_SIZE;
	float gaugeRaito : GAUGE_RAITO;
};

struct GSOutput
{
	float4 svpos : SV_POSITION; // システム用頂点座標
	float2 uv : TEXCOORD; // uv値
	float4 color : COLOR;
	float holeSize : HOLE_SIZE;
	float gaugeRaito : GAUGE_RAITO;
};

// レンダーターゲットの数は2つ
struct PSOutput
{
	float4 target0 : SV_TARGET0;
	float4 target1 : SV_TARGET1;
};