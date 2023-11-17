#include "SpriteBase.h"

#pragma comment(lib, "d3d12.lib")

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <d3dx12.h>

#include <DirectXTex.h>
#include "System/DX12Base.h"

#include <Texture/Texture.h>

using namespace DirectX;
using namespace Microsoft::WRL;

UINT SpriteBase::nowTexNum = 0u;

// スプライト用パイプライン生成
SpriteBase::PipelineSet SpriteBase::SpriteCreateGraphicsPipeline(ID3D12Device* dev,
																 const wchar_t* vsPath, const wchar_t* psPath,
																 BLEND_MODE blendMode)
{
	ComPtr<ID3DBlob> vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob = nullptr; // エラーオブジェクト

	constexpr UINT compileFlag =
#ifdef _DEBUG
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		0;
#endif // _DEBUG

	// 頂点シェーダの読み込みとコンパイル
	HRESULT result = D3DCompileFromFile(
		vsPath,  // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
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
		psPath,   // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
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

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};

	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;              // 背面カリングをしない

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = gpipeline.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // 標準設定
	blenddesc.BlendEnable = true;                   // ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;    // 加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;      // ソースの値を100% 使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;    // デストの値を   0% 使う

	switch (blendMode)
	{
	case SpriteBase::BLEND_MODE::ADD:
		//---加算
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;			//加算
		blenddesc.SrcBlend = D3D12_BLEND_ONE;				//ソースの値を100%使う
		blenddesc.DestBlend = D3D12_BLEND_ONE;			//デストの値を100%使う
		break;
	case SpriteBase::BLEND_MODE::SUB:
		//---減算
		blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;	//デストからソースを減算
		blenddesc.SrcBlend = D3D12_BLEND_ONE;				//ソースの値を100%使う
		blenddesc.DestBlend = D3D12_BLEND_ONE;			//デストの値を100%使う
		break;
	case SpriteBase::BLEND_MODE::REVERSE:
		//---反転
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;			//加算
		blenddesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;	//1.0 - デストカラーの値
		blenddesc.DestBlend = D3D12_BLEND_ZERO;
		break;
	default:
		//--半透明合成
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;				//加算
		blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//ソースのアルファ値
		blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	//デストの値を100%使う
		break;
	}

	// デプスステンシルステートの設定
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DepthStencilState.DepthEnable = false;    // 深度テストをしない
	//gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;       // 常に上書きルール
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT; // 深度値フォーマット

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1; // 描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0～255指定のRGBA
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	// デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	// ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[2]{};
	rootparams[0].InitAsConstantBufferView(0); // 定数バッファビューとして初期化(b0レジスタ)
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; // ニアレストネイバー法

	// パイプラインとルートシグネチャのセット
	SpriteBase::PipelineSet pipelineSet{};

	// ルートシグネチャの生成
	// ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定でのシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	// ルートシグネチャの生成
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pipelineSet.rootsignature));

	// パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = pipelineSet.rootsignature.Get();

	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineSet.pipelinestate));

	// パイプラインとルートシグネチャを返す
	return pipelineSet;
}

// スプライト共通テクスチャ読み込み
UINT SpriteBase::loadTexture(const wchar_t* filename, DirectX::XMFLOAT2* pTexSize)
{
	// 異常な番号の指定を検出
	assert(nowTexNum + 1 <= SpriteBase::spriteSRVCount - 1);

	++nowTexNum;

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	// WICテクスチャのロード
	Texture::loadTexFile(filename, metadata, scratchImg);

	// テクスチャバッファ生成
	auto descHandleIncSize = DX12Base::ins()->getDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), nowTexNum, descHandleIncSize);
	Texture::createTexBuff(metadata, scratchImg, texBuff[nowTexNum], cpuHandle);

	// 画像サイズ記録
	if (pTexSize != nullptr) { *pTexSize = XMFLOAT2(float(metadata.width), float(metadata.height)); }

	return nowTexNum;
}

// スプライト共通グラフィックコマンドのセット
void SpriteBase::drawStart(ID3D12GraphicsCommandList* cmdList)
{
	// パイプラインステートの設定
	cmdList->SetPipelineState(pipelineSet.pipelinestate.Get());
	// ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(pipelineSet.rootsignature.Get());
	// プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// テクスチャ用デスクリプタヒープの設定
	ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

SpriteBase::SpriteBase(BLEND_MODE blendMode,
					   const wchar_t* vsPath, const wchar_t* psPath)
{
	// スプライト用パイプライン生成
	pipelineSet = SpriteCreateGraphicsPipeline(DX12Base::getInstance()->getDev(), vsPath, psPath, blendMode);

	// 並行投影の射影行列生成
	matProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, (float)WinAPI::window_width, (float)WinAPI::window_height, 0.0f, 0.0f, 1.0f);

	// デスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = SpriteBase::spriteSRVCount;
	HRESULT result = DX12Base::getInstance()->getDev()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));
}