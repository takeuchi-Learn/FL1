#pragma once

#include <DirectXMath.h>

class PointLight
{
public:
	struct ConstBufferData
	{
		DirectX::XMFLOAT3 pos{};
		float pad1{};
		DirectX::XMFLOAT3 color{};
		float pad2{};
		DirectX::XMFLOAT3 atten{};
		float pad3{};
	};

private:
	// 有効かどうか
	bool active = false;

	// 位置
	DirectX::XMFLOAT3 pos{};

	// ライト色
	DirectX::XMFLOAT3 color{ 1, 1, 1 };

	// 距離減衰係数
	DirectX::XMFLOAT3 atten{ 0.3f, 0, 0 };

public:
	inline void setActive(bool active) { this->active = active; }
	inline bool getActive() const { return active; }

	inline void setPos(const DirectX::XMFLOAT3& pos) { this->pos = pos; }
	inline const DirectX::XMFLOAT3& getPos() const { return pos; }

	inline void setColor(const DirectX::XMFLOAT3& color) { this->color = color; }
	inline const DirectX::XMFLOAT3& getColor() const { return color; }

	inline void setAtten(const DirectX::XMFLOAT3& atten) { this->atten = atten; }
	inline const DirectX::XMFLOAT3& getAtten() const { return atten; }
};
