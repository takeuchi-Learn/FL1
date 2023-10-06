#include "CollisionMgr.h"
#include <Collision/CollisionShape.h>
#include <Collision/Collision.h>
#include <DirectXMath.h>

using namespace DirectX;

void CollisionMgr::checkHitAll(const ColliderSet& collider1,
							   const ColliderSet& collider2)
{
	if (collider1.group.empty() || collider2.group.empty()) { return; }

	for (auto& g1 : collider1.group)
	{
		if (!g1.obj) { continue; }
		if (!g1.obj->isAlive()) { continue; }

		for (auto& g2 : collider2.group)
		{
			if (!g2.obj) { continue; }
			if (!g2.obj->isAlive()) { continue; }

			XMVECTOR g1Pos = XMLoadFloat3(&g1.obj->getObj()->calcWorldPos());
			XMVECTOR g2Pos = XMLoadFloat3(&g2.obj->getObj()->calcWorldPos());

			if (Collision::CheckHit(CollisionShape::Capsule(g1Pos, g1Pos, g1.colliderR),
									CollisionShape::Capsule(g2Pos, g2Pos, g2.colliderR)))
			{
				collider1.hitProc(g1.obj);
				collider2.hitProc(g2.obj);
			}
		}
	}
}