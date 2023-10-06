#pragma once

#include <DirectXMath.h>

class CircleShadow
{
public:
	struct ConstBufferData
	{
		DirectX::XMVECTOR invLightDirNormal;
		DirectX::XMFLOAT3 casterPos;
		float caster2LightDistance;
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

	DirectX::XMVECTOR dir{ 0, -1, 0, 0 };
	DirectX::XMFLOAT3 casterPos{ 0, 0, 0 };
	float caster2LightDistance = 100.f;
	DirectX::XMFLOAT3 atten{ 0.5f, 0.6f, 0.f };
	DirectX::XMFLOAT2 factorAngleCos{ 1.f, 0.9996f };
};
