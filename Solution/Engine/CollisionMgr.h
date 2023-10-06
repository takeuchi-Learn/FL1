/*****************************************************************//**
 * \file   CollisionMgr.h
 * \brief  衝突判定をするクラス。現状では球体のみ対応している。
 *********************************************************************/

#pragma once
#include <functional>
#include <forward_list>
#include <GameObject/AbstractGameObj.h>
#include <Collision/CollisionShape.h>
#include <DirectXMath.h>
#include <array>

 /// @brief 衝突判定をするクラス
class CollisionMgr
{
public:
	struct ColliderType
	{
		AbstractGameObj* obj = nullptr;
		float colliderR = 1.f;
		std::array<DirectX::XMFLOAT3, 2u> pos;

		inline static ColliderType create(AbstractGameObj* obj,
										  float colliderR,
										  const DirectX::XMFLOAT3& pos1 = DirectX::XMFLOAT3(),
										  const DirectX::XMFLOAT3& pos2 = DirectX::XMFLOAT3())
		{
			auto ret = ColliderType{ .obj = obj, .colliderR = colliderR };
			ret.pos[0] = pos1;
			ret.pos[1] = pos2;
			return ret;
		}
	};

	using GroupType = std::forward_list<ColliderType>;

	struct ColliderSet
	{
		/// @brief コライダーグループ
		GroupType group;

		/// @brief 衝突したときに行う処理
		std::function<void(AbstractGameObj*)> hitProc = [](AbstractGameObj*) {};
	};

	/// @brief 二つのコライダーセット全ての衝突判定をする
	/// @param collider1 コライダーのセット1
	/// @param collider2 コライダーのセット2
	static void checkHitAll(const ColliderSet& collider1,
							const ColliderSet& collider2);
};
