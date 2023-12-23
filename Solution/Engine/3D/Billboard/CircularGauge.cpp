#include "CircularGauge.h"
#include <System/PostEffect.h>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

namespace
{
	auto* dxBase = DX12Base::ins();

	constexpr auto vsPath = L"Resources/Shaders/CircularGaugeVS.hlsl";
	constexpr auto psPath = L"Resources/Shaders/CircularGaugePS.hlsl";
	constexpr auto gsPath = L"Resources/Shaders/CircularGaugeGS.hlsl";
}

CircularGauge::CircularGauge(Camera* camera)
	: camera(camera)
{
	initDescHeap();
	initGraphPipeline();
	createModel();
	createConstBuff();
}

void CircularGauge::update()
{
	Vertex* vertMap = nullptr;
	if (SUCCEEDED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		uint16_t vertCount = 0ui16;
		for (auto& i : gauges)
		{
			vertMap->pos = i->position;
			vertMap->color = i->color;
			vertMap->scale = i->scale;
			vertMap++;
			if (++vertCount >= vertexCount)
			{
				break;
			}
		}
		vertBuff->Unmap(0, nullptr);
	}

	// 定数バッファへデータ転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->mat = camera->getViewProjectionMatrix();
	constMap->matBillboard = camera->getBillboardMatrix();
	constBuff->Unmap(0, nullptr);
}

void CircularGauge::draw()
{
	if (drawNum == 0ui16) { return; }
	drawNum = std::min(drawNum, vertexCount);

	// パイプラインステートの設定
	dxBase->getCmdList()->SetPipelineState(pipelinestate[static_cast<size_t>(nowBlendMode)].Get());
	// ルートシグネチャの設定
	dxBase->getCmdList()->SetGraphicsRootSignature(rootsignature.Get());
	// プリミティブ形状を設定
	dxBase->getCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	// 頂点バッファの設定
	dxBase->getCmdList()->IASetVertexBuffers(0, 1, &vbView);
	// デスクリプタヒープの配列
	ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
	dxBase->getCmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// 定数バッファビューをセット
	dxBase->getCmdList()->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());
	// 描画コマンド
	dxBase->getCmdList()->DrawInstanced(drawNum, 1, 0, 0);
}
std::weak_ptr<CircularGaugeData> CircularGauge::add(const XMFLOAT3& pos, float scale, const XMFLOAT4& color)
{
	if (drawNum >= vertexCount)
	{
		return std::weak_ptr<CircularGaugeData>();
	}
	auto& i = gauges.emplace_front(std::make_shared<CircularGaugeData>());
	++drawNum;

	i->position = pos;
	i->scale = scale;
	i->color = color;

	return i;
}

void CircularGauge::initDescHeap()
{
	// デスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NumDescriptors = 1; // シェーダーリソースビュー1つ
	HRESULT result = dxBase->getDev()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));//生成
	assert(SUCCEEDED(result));

	// デスクリプタサイズを取得
	descriptorHandleIncrementSize = dxBase->getDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CircularGauge::initGraphPipeline()
{
	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;	// ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> gsBlob;	// ジオメトリシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

	constexpr UINT compileFlag =
#ifdef _DEBUG
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		0;
#endif // _DEBUG

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		vsPath,	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定
		compileFlag,
		0,
		&vsBlob, &errorBlob);
	if (FAILED(result))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		psPath,	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定
		compileFlag,
		0,
		&psBlob, &errorBlob);
	if (FAILED(result))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ジオメトリシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		gsPath,	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "gs_5_0",	// エントリーポイント名、シェーダーモデル指定
		compileFlag,
		0,
		&gsBlob, &errorBlob);
	if (FAILED(result))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy座標(1行で書いたほうが見やすい)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // スケール
			"TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // 色
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

#pragma region ブレンド設定

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// RBGA全てのチャンネルを描画
	blenddesc.BlendEnable = true;

#pragma region ブレンドステート

	//半透明合成
	/*blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;*/
	//加算
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_ONE;
	blenddesc.DestBlend = D3D12_BLEND_ONE;
	////減算
	//blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	//blenddesc.SrcBlend = D3D12_BLEND_ONE;
	//blenddesc.DestBlend = D3D12_BLEND_ONE;

#pragma endregion ブレンドステート

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

#pragma endregion ブレンド設定

	// グラフィックスパイプラインの流れを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	gpipeline.GS = CD3DX12_SHADER_BYTECODE(gsBlob.Get());

	// サンプルマスク
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	// ラスタライザステート
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// デプスステンシルステート
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	// デプスの書き込みを禁止
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	constexpr UINT renderTargetNum = std::min(PostEffect::renderTargetNum, UINT(_countof(gpipeline.BlendState.RenderTarget)));

	// ブレンドステートの設定
	for (UINT i = 0u; i < renderTargetNum; ++i)
	{
		gpipeline.BlendState.RenderTarget[i] = blenddesc;
	}

	// 深度バッファのフォーマット
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// 頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定（三角形）
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	gpipeline.NumRenderTargets = renderTargetNum;	// 描画対象の数
	for (UINT i = 0u; i < renderTargetNum; ++i)
	{
		gpipeline.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA
	}
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	gpipeline.BlendState.AlphaToCoverageEnable = true;	//透明部分の深度値は書き込まない

	// デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	// ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootparams[2]{};
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; // ニアレストネイバー法

	// ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	// ルートシグネチャの生成
	result = dxBase->getDev()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	assert(SUCCEEDED(result));

	gpipeline.pRootSignature = rootsignature.Get();

	// グラフィックスパイプラインの生成
	result = dxBase->getDev()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate[(size_t)BLENDMODE::ADD]));
	assert(SUCCEEDED(result));

#pragma region 減算合成のパイプライン生成

	// 減算
	blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	blenddesc.SrcBlend = D3D12_BLEND_ONE;
	blenddesc.DestBlend = D3D12_BLEND_ONE;

	// ブレンドステートの設定
	for (UINT i = 0u; i < renderTargetNum; ++i)
	{
		gpipeline.BlendState.RenderTarget[i] = blenddesc;
	}

	result = dxBase->getDev()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate[(size_t)BLENDMODE::SUB]));
	assert(SUCCEEDED(result));

#pragma endregion 減算合成のパイプライン生成

#pragma region 半透明合成のパイプライン生成

	// 半透明
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	// ブレンドステートの設定
	for (UINT i = 0u; i < renderTargetNum; ++i)
	{
		gpipeline.BlendState.RenderTarget[i] = blenddesc;
	}

	result = dxBase->getDev()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate[(size_t)BLENDMODE::ALPHA]));
	assert(SUCCEEDED(result));

#pragma endregion 半透明合成のパイプライン生成
}

void CircularGauge::createModel()
{
	// 頂点バッファ生成
	HRESULT result = dxBase->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * vertexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(result))
	{
		assert(0);
		return;
	}

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(Vertex) * vertexCount;
	vbView.StrideInBytes = sizeof(Vertex);
}

void CircularGauge::createConstBuff()
{
	// 定数バッファの生成
	HRESULT result = dxBase->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	assert(SUCCEEDED(result));
}