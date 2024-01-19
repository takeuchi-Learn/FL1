#include "Billboard.hlsli"

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

// ループ用
static uint i;

//点の入力から、四角形を出力
[maxvertexcount(vnum)]
void main(
	//出力プリミティブの種類(受け取る形状)
	point VSOutput input[1] : SV_POSITION,
	inout TriangleStream<GSOutput> output
	//     出力ストリーム型(出力する形状)
)
{
	static float sinAngle = 0.f;
	static float cosAngle = 0.f;
	static float4 offset = float4(0.f, 0.f, 0.f, 0.f);
	
	sinAngle = sin(angleRad);
	cosAngle = cos(angleRad);
	GSOutput element;
	//4点分回す
	for (i = 0; i < vnum; i++)
	{
		//中心からのオフセットを回転スケーリング
		offset.x = input[0].scale.x * (center.x / 2.f + offset_array[i].x) * cosAngle - input[0].scale.y * (center.y / 2.f + offset_array[i].y) * sinAngle;
		offset.y = input[0].scale.y * (center.y / 2.f + offset_array[i].y) * cosAngle + input[0].scale.x * (center.x / 2.f + offset_array[i].x) * sinAngle;
		offset.zw = offset_array[i].zw;
		//中心からのオフセットをビルボード回転(モデル座標)
		offset = mul(matBillboard, offset);
		//オフセット分ずらす(ワールド座標)
		element.svpos = input[0].pos + offset;
		//ビュー、射影変換
		element.svpos = mul(mat, element.svpos);
		//element.uv = float2(0.5f, 0.5f);
		element.uv = uv_array[i];
		element.color = input[0].color;
		output.Append(element);
	}
}