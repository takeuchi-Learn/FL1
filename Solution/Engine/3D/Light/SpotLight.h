#pragma once

#include <DirectXMath.h>

class SpotLight
{
public:
	struct ConstBufferData
	{
		DirectX::XMVECTOR invLightDirNormal;
		DirectX::XMFLOAT3 pos;
		float pad1;
		DirectX::XMFLOAT3 color;
		float pad2;
		DirectX::XMFLOAT3 atten;
		float pad3;
		DirectX::XMFLOAT2 factorAngleCos;
		DirectX::XMFLOAT2 pad4;
	};

private:
	bool active = false;

public:

	inline void setActive(bool active) { this->active = active; }
	inline bool getActive() const { return active; }

	DirectX::XMVECTOR dir{ 1, 0, 0, 0 };
	DirectX::XMFLOAT3 pos{};
	DirectX::XMFLOAT3 color{ 1, 1, 1 };
	DirectX::XMFLOAT3 atten{ 0.1f, 0.1f, 0 };	// 距離減衰係数
	DirectX::XMFLOAT2 factorAngleCos{ 0.998f, 0.994f };	// 減衰角度のコサイン(x=開始角度、y=終了角度)
};
