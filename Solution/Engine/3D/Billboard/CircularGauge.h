#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <forward_list>
#include "Camera/Camera.h"

class CircularGaugeData
{
public:
	DirectX::XMFLOAT3 position{};
	float scale = 1.f;
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
};

class CircularGauge
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

public:
	struct Vertex
	{
		XMFLOAT3 pos;
		float scale;
		XMFLOAT4 color;
	};

	// 定数バッファ用データ構造体
	struct ConstBufferData
	{
		XMMATRIX mat;	// ビュープロジェクション行列
		XMMATRIX matBillboard;	// ビルボード行列
	};

private:
	static constexpr uint16_t vertexCount = UINT16_MAX;

private:
	uint16_t drawNum = 0ui16;

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
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView;
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	std::forward_list<std::shared_ptr<CircularGaugeData>> gauges;
	Camera* camera = nullptr;

public:
	/// @brief 現在のブレンドモード
	BLENDMODE nowBlendMode = BLENDMODE::ALPHA;

public:
	CircularGauge() : CircularGauge(nullptr) {}
	CircularGauge(Camera* camera);
	void update();
	void draw();

	std::weak_ptr<CircularGaugeData> add(const XMFLOAT3& pos,
										 float scale,
										 const XMFLOAT4& color = XMFLOAT4(1, 1, 1, 1));

	inline void setCamera(Camera* camera) { this->camera = camera; }

private:
	void initDescHeap();
	void initGraphPipeline();
	void createModel();
	void createConstBuff();
};
