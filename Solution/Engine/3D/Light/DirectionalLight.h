#pragma once

#include <DirectXMath.h>

class DirectionalLight
{
public:
	struct ConstBufferData
	{
		DirectX::XMVECTOR dir2Light{};
		DirectX::XMFLOAT3 color{};
		float pad{};
	};

private:
	bool active = false;

	DirectX::XMVECTOR dir = { 0, -1, 0, 0 };
	DirectX::XMFLOAT3 color{ 1, 1, 1 };

public:
	inline void setActive(bool active) { this->active = active; }
	inline bool getActive() const { return active; }

	inline void setDir(const DirectX::XMVECTOR& dir) { this->dir = dir; }
	inline const auto& getDir() const { return dir; }

	inline void setColor(const DirectX::XMFLOAT3& color) { this->color = color; }
	inline const auto& getColor() const { return color; }
};