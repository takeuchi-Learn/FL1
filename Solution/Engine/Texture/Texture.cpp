#include "Texture.h"
#include <System/DX12Base.h>

using namespace DirectX;
using namespace Microsoft::WRL;

bool Texture::loadTexFile(const wchar_t* wfilePath,
						  TexMetadata& metadata,
						  ScratchImage& image,
						  WIC_FLAGS wicFlags,
						  TEX_FILTER_FLAGS mipMapFlags)
{
	HRESULT result = LoadFromWICFile(wfilePath,
									 wicFlags,
									 &metadata,
									 image);
	// 読み込みに失敗したらtrueを返す
	if (FAILED(result))
	{
		return true;
	}

	ScratchImage mipChain{};
	// ミップマップ生成
	result = GenerateMipMaps(image.GetImages(),
							 image.GetImageCount(),
							 image.GetMetadata(),
							 mipMapFlags,
							 0,
							 mipChain);

	if (SUCCEEDED(result))
	{
		image = std::move(mipChain);
		metadata = image.GetMetadata();
	}

	// 読み込んだテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	return false;
}

bool Texture::createTexBuff(TexMetadata& metadata,
							ScratchImage& image,
							ComPtr<ID3D12Resource>& texBuff,
							CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// テクスチャ用バッファの生成
	HRESULT result = DX12Base::ins()->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texBuff));
	if (FAILED(result))
	{
		assert(0);
		return true;
	}

	for (size_t i = 0; i < metadata.mipLevels; ++i)
	{
		const Image* img = image.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff->WriteToSubresource(
			(UINT)i,
			nullptr,				// 全領域へコピー
			img->pixels,			// 元データアドレス
			(UINT)img->rowPitch,	// 1ラインサイズ
			(UINT)img->slicePitch	// 1枚サイズ
		);
		if (FAILED(result))
		{
			assert(0);
			return true;
		}
	}

	// シェーダリソースビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texBuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	DX12Base::ins()->getDev()->CreateShaderResourceView(texBuff.Get(), //ビューと関連付けるバッファ
														&srvDesc, //テクスチャ設定情報
														cpuHandle);

	return false;
}

bool Texture::createMonoColorTexBuff(uint32_t color,
									 ComPtr<ID3D12Resource>& texBuff,
									 CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
									 1u,
									 1u,
									 1ui16,
									 1ui16);

	// テクスチャ用バッファの生成
	HRESULT result = DX12Base::ins()->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texBuff));
	if (FAILED(result))
	{
		assert(0);
		return true;
	}

	auto tex = std::make_unique<uint32_t>(color);
	// テクスチャバッファにデータ転送
	result = texBuff->WriteToSubresource(
		0u,
		nullptr,			// 全領域へコピー
		tex.get(),			// 元データアドレス
		sizeof(uint32_t),	// 1ラインサイズ
		sizeof(uint32_t)	// 1枚サイズ
	);
	tex.reset(nullptr);
	if (FAILED(result))
	{
		assert(0);
		return true;
	}

	// シェーダリソースビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texBuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	DX12Base::ins()->getDev()->CreateShaderResourceView(texBuff.Get(), //ビューと関連付けるバッファ
														&srvDesc, //テクスチャ設定情報
														cpuHandle);

	return false;
}