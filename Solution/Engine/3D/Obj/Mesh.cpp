#include "Mesh.h"
#include <d3dcompiler.h>
#include <cassert>
#include <System/DX12Base.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

void Mesh::transferVertBuf()
{
	VertexPosNormalUv* vertMap = nullptr;
	if (SUCCEEDED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		std::copy(vertices.begin(), vertices.end(), vertMap);
		vertBuff->Unmap(0, nullptr);
	}
}

void Mesh::transferIndexBuf()
{
	unsigned short* indexMap = nullptr;
	if (SUCCEEDED(indexBuff->Map(0, nullptr, (void**)&indexMap)))
	{
		std::copy(indices.begin(), indices.end(), indexMap);
		indexBuff->Unmap(0, nullptr);
	}
}

void Mesh::createBuffers()
{
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv) * vertices.size());
	// 頂点バッファ生成
	HRESULT result = DX12Base::ins()->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	// 頂点バッファへのデータ転送
	transferVertBuf();

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	if (FAILED(result))
	{
		assert(0);
		return;
	}

	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	// インデックスバッファ生成
	result = DX12Base::ins()->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));
	if (FAILED(result))
	{
		assert(0);
		return;
	}

	// インデックスバッファへのデータ転送
	transferIndexBuf();

	// インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
}

void Mesh::draw(ID3D12GraphicsCommandList* cmdList)
{
	// 頂点バッファをセット
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	// インデックスバッファをセット
	cmdList->IASetIndexBuffer(&ibView);
	// シェーダリソースビューをセット
	cmdList->SetGraphicsRootDescriptorTable(2, material->GetGpuHandle());

	// マテリアルの定数バッファをセット
	ID3D12Resource* constBuff = material->getConstBuff();
	cmdList->SetGraphicsRootConstantBufferView(1, constBuff->GetGPUVirtualAddress());

	// 描画コマンド
	cmdList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
}

void Mesh::calculateSmoothedVertexNormals()
{
	for (auto& i : smoothData)
	{
		// 全頂点の法線を平均する
		XMVECTOR normal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		for (unsigned short index : i.second)
		{
			normal += XMLoadFloat3(&vertices[index].normal);
		}
		normal = XMVector3Normalize(normal / static_cast<float>(i.second.size()));

		// 共通法線を使用する全頂点データに書き込む
		for (unsigned short index : i.second)
		{
			XMStoreFloat3(&vertices[index].normal, normal);
		}
	}
}