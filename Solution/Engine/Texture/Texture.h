#pragma once

#include <DirectXTex.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>

class Texture
{
public:
	/// @return エラーの有無（正常終了でfalse）
	static bool loadTexFile(const wchar_t* wfilePath,
							DirectX::TexMetadata& metadata,
							DirectX::ScratchImage& image,
							DirectX::WIC_FLAGS wicFlags = DirectX::WIC_FLAGS::WIC_FLAGS_NONE,
							DirectX::TEX_FILTER_FLAGS mipMapFlags = DirectX::TEX_FILTER_FLAGS::TEX_FILTER_DEFAULT);

	static bool createTexBuff(DirectX::TexMetadata& metadata,
							  DirectX::ScratchImage& image,
							  Microsoft::WRL::ComPtr<ID3D12Resource>& texBuff,
							  CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle);

	/// @brief 指定色1x1でテクスチャバッファを作成
	/// @param color 色（BE）
	static bool createMonoColorTexBuff(uint32_t color,
									   Microsoft::WRL::ComPtr<ID3D12Resource>& texBuff,
									   CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle);
};
