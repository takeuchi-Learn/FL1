#include "CircularGauge.hlsli"

//四角形の頂点数
static const uint vnum = 4;

//センター空のオフセット
static const float4 offset_array[vnum] =
{
	float4(-0.5f, -0.5f, 0, 0), //左下
	float4(-0.5f, +0.5f, 0, 0), //左上
	float4(+0.5f, -0.5f, 0, 0), //右下
	float4(+0.5f, +0.5f, 0, 0), //右上
};

//左上が0,0 右下が,1
static const float2 uv_array[vnum] =
{
	float2(0, 1), //左下
	float2(0, 0), //左上
	float2(1, 1), //右下
	float2(1, 0), //右上
};

//点の入力から、四角形を出力
[maxvertexcount(vnum)]
void main(
	//出力プリミティブの種類(受け取る形状)
	point VSOutput input[1] : SV_POSITION,
	inout TriangleStream<GSOutput> output
	//     出力ストリーム型(出力する形状)
)
{
	GSOutput element;
	//4点分回す
	for (uint i = 0; i < vnum; i++)
	{
		//中心からのオフセットをスケーリング
		float4 offset = offset_array[i] * input[0].scale;
		//中心からのオフセットをビルボード回転(モデル座標)
		offset = mul(matBillboard, offset);
		//オフセット分ずらす(ワールド座標)
		element.svpos = input[0].pos + offset;
		//ビュー、射影変換
		element.svpos = mul(mat, element.svpos);
		//element.uv = float2(0.5f, 0.5f);
		element.uv = uv_array[i];
		element.color = input[0].color;
		element.holeSize = input[0].holeSize;
		element.gaugeRaito = input[0].gaugeRaito;
		output.Append(element);
	}
}