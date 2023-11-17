#include "Material.h"
#include <DirectXTex.h>
#include <cassert>
#include <System/DX12Base.h>
#include <Texture/Texture.h>

using namespace DirectX;

Material::Material()
	: ambient({ 0.3f, 0.3f, 0.3f }),
	diffuse({ 0.f, 0.f, 0.f }),
	specular({ 0.5f, 0.5f, 0.5f }),
	alpha(1.f),
	texFileName("<EMPTY>")
{
	createConstBuff();
	texbuff.resize(Material::maxTexNum);
}

void Material::loadTexture(const std::string& directoryPath, UINT texNum,
						   CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
						   CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
	cpuDescHandleSRV = cpuHandle;
	gpuDescHandleSRV = gpuHandle;

	// WICテクスチャのロード
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	const std::string filepath = directoryPath + texFileName;

	constexpr size_t wfilePathSize = 512u;
	wchar_t wfilepath[wfilePathSize]{};
	MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, wfilePathSize);

	if (Texture::loadTexFile(wfilepath, metadata, scratchImg))
	{
		Texture::createMonoColorTexBuff(UINT32_MAX, texbuff[texNum], cpuDescHandleSRV);
	} else
	{
		Texture::createTexBuff(metadata, scratchImg, texbuff[texNum], cpuDescHandleSRV);
	}
}

void Material::loadMonoColorTexture(UINT texNum, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
	cpuDescHandleSRV = cpuHandle;
	gpuDescHandleSRV = gpuHandle;
	Texture::createMonoColorTexBuff(UINT32_MAX, texbuff[texNum], cpuDescHandleSRV);
}

void Material::update()
{
	// 定数バッファへデータ転送
	ConstBufferDataB1* constMap = nullptr;
	HRESULT result = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result))
	{
		constMap->ambient = ambient;
		constMap->diffuse = diffuse;
		constMap->specular = specular;
		constMap->alpha = alpha;
		constMap->texTilling = texTilling;
		constMap->shiftUv = shiftUv;
		constBuff->Unmap(0, nullptr);
	}
}

void Material::createConstBuff()
{
	// 定数バッファの生成
	HRESULT result = DX12Base::ins()->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB1) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	assert(SUCCEEDED(result));
}