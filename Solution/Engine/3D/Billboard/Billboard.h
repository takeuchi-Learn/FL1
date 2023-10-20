#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <forward_list>
#include "Camera/Camera.h"

// ビルボード一枚
class BillboardData
{
public:
	// 座標
	DirectX::XMFLOAT3 position = {};
	// 色
	DirectX::XMFLOAT4 color = { 1, 1, 1, 1 };
	// スケール
	float scale = 1.f;
	// 回転
	float rotation = 0.f;

	// 要素を削除するかどうか
	bool deleteFlag = false;
};

class Billboard
{
public:
	enum class BLENDMODE : uint8_t
	{
		ADD = 0u,
		SUB,
		ALPHA,

		/// @brief 総数
		/// @note これより下に要素を追加してはいけない
		ALLCOUNT
	};

private:
	// エイリアス
   // Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

	// サブクラス
public:
	// 頂点データ構造体
	struct VertexPos
	{
		XMFLOAT3 pos; // xyz座標
		float scale; // スケール
		XMFLOAT4 color;
	};

	// 定数バッファ用データ構造体
	struct ConstBufferData
	{
		XMMATRIX mat;	// ビュープロジェクション行列
		XMMATRIX matBillboard;	// ビルボード行列
	};

	// 定数
private:
	static DX12Base* dxBase;

	static const uint32_t vertexCount = 0x10000u;

	// メンバ変数
private:
	// デスクリプタサイズ
	UINT descriptorHandleIncrementSize = 0U;
	// ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootsignature;
	// パイプラインステートオブジェクト
	ComPtr<ID3D12PipelineState> pipelinestate[size_t(BLENDMODE::ALLCOUNT)];
	// デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeap;
	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	// テクスチャバッファ
	ComPtr<ID3D12Resource> texbuff;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView;
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	// パーティクル配列
	std::forward_list<std::shared_ptr<BillboardData>> billboards;
	Camera* camera = nullptr;

	uint32_t drawNum = 0u;

public:
	/// @brief 現在のブレンドモード
	BLENDMODE nowBlendMode = BLENDMODE::ALPHA;

	// メンバ関数
public:
	// テクスチャは1x1白で初期化
	Billboard();

	// 指定の画像で初期化
	Billboard(const wchar_t* texFilePath, Camera* camera);

	void update();
	void draw();

	inline void setCamera(Camera* camera) { this->camera = camera; }

	std::weak_ptr<BillboardData> add(const XMFLOAT3& position,
									 float scale,
									 float rotation = 0.f,
									 const XMFLOAT4& color = XMFLOAT4(1.f, 1.f, 1.f, 1.f));

	/// @brief デスクリプタヒープの初期化
	void InitializeDescriptorHeap();

	/// @brief グラフィックパイプライン生成
	void InitializeGraphicsPipeline();

	/// @brief テクスチャ読み込み
	/// @param filePath テクスチャファイルのパス
	void LoadTexture(const wchar_t* filePath);

	/// @brief モデル作成
	void CreateModel();
};
