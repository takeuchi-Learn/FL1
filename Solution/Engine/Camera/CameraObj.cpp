#include "CameraObj.h"
#include <algorithm>

using namespace DirectX;

CameraObj::CameraObj(AbstractGameObj* parent)
	:Camera(WinAPI::window_width,
			WinAPI::window_height),
	parentObj(parent)
{}

void CameraObj::rotaCameraPos(const DirectX::XMFLOAT2& inputDeg)
{
	relativeRotaDeg.x += inputDeg.y;
	relativeRotaDeg.y += inputDeg.x;

	constexpr float vAngleMax = 80.f;
	relativeRotaDeg.x = std::clamp(relativeRotaDeg.x, 0.f, vAngleMax);
	relativeRotaDeg.y = std::fmod(relativeRotaDeg.y, 360.f);
}

void CameraObj::updateMatWorld()
{
	XMFLOAT3 distance{}, rota{};
	XMMATRIX matTrans{};

	XMFLOAT3 eye = getEye(), target = getTarget();

	distance = XMFLOAT3(target.x - eye.x,
						target.y - eye.y,
						target.z - eye.z);

	if (useParentRotaFlag)
	{
		rota = parentObj->getRotation();
		for (auto* parent = parentObj->getParent();
			 parent;
			 parent = parent->parent)
		{
			rota.x += parent->rotation.x;
			rota.y += parent->rotation.y;
			rota.z += parent->rotation.z;
		}
	}

	matRot = XMMatrixRotationZ(XMConvertToRadians(rota.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rota.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rota.y));

	matTrans = XMMatrixTranslation(eye.x, eye.y, eye.z);

	matWorld = matRot;
	matWorld *= matTrans;
}

void CameraObj::preUpdate()
{
	if (parentObj == nullptr) { return; }

	// カメラ操作を反転するかどうか
	constexpr bool invCamOperFlag = false;
	// 反転するなら1、しないなら-1
	constexpr float rotaInvFactor = invCamOperFlag ? 1.f : -1.f;

	XMFLOAT3 targetPos = parentObj->getObj()->calcWorldPos();

	XMFLOAT3 rotaAngle = relativeRotaDeg;

	if (useParentRotaFlag)
	{
		rotaAngle.x += parentObj->getRotation().x;
		rotaAngle.y += parentObj->getRotation().y;
		rotaAngle.z += parentObj->getRotation().z;
		{
			for (auto* parent = parentObj->getObj()->parent;
				 parent;
				 parent = parent->parent)
			{
				rotaAngle.x += parent->rotation.x;
				rotaAngle.y += parent->rotation.y;
				rotaAngle.z += parent->rotation.z;
			}
		}
	}

	// 回転を取得[rad]
	const XMFLOAT3 rotaAngleRad{
		XMConvertToRadians(rotaInvFactor * rotaAngle.x),
		XMConvertToRadians(rotaInvFactor * rotaAngle.y),
		XMConvertToRadians(rotaInvFactor * rotaAngle.z)
	};

	// 垂直角度を計算
	float sinT = std::sin(-rotaAngleRad.x);
	float cosT = std::cos(-rotaAngleRad.x);

	const XMFLOAT3 verticalPos{
		0.f,
		sinT * eye2TargetLen,
		-cosT * eye2TargetLen
	};

	// 水平角度を計算
	sinT = std::sin(rotaAngleRad.y);
	cosT = std::cos(rotaAngleRad.y);
	const XMFLOAT3 horizontalPos{
		cosT * verticalPos.x - sinT * verticalPos.z,
		verticalPos.y,
		sinT * verticalPos.x + cosT * verticalPos.z
	};

	// 計算した座標 + 注視点の位置 = カメラの位置
	XMFLOAT3 eye{
		horizontalPos.x + targetPos.x,
		horizontalPos.y + targetPos.y,
		horizontalPos.z + targetPos.z,
	};

	// 前回のカメラ位置
	const XMFLOAT3 oldEye = getEye();

	// 補間割合を加味したカメラ移動幅
	eye = {
		(eye.x - oldEye.x) * easeRaito,
		(eye.y - oldEye.y) * easeRaito,
		(eye.z - oldEye.z) * easeRaito
	};

	// 前回の位置に移動幅を足す
	eye = {
		oldEye.x + eye.x,
		oldEye.y + eye.y,
		oldEye.z + eye.z
	};

	// 注視点の位置を高くする
	{
		XMFLOAT3 camHeiVec{};
		if (useParentRotaFlag)
		{
			XMStoreFloat3(&camHeiVec, XMVector3Transform(XMVectorSet(eye2TargetOffset.x,
																	 eye2TargetOffset.y,
																	 eye2TargetOffset.z,
																	 1),
														 parentObj->getObj()->getMatRotation()));
		} else
		{
			camHeiVec = eye2TargetOffset;
		}
		targetPos.x += camHeiVec.x;
		targetPos.y += camHeiVec.y;
		targetPos.z += camHeiVec.z;
	}

	setEye(eye);
	setTarget(targetPos);

	updateMatWorld();

	{
		const XMVECTOR up = XMVectorSet(matWorld.r[1].m128_f32[0],
										matWorld.r[1].m128_f32[1],
										matWorld.r[1].m128_f32[2],
										matWorld.r[1].m128_f32[3]);

		XMFLOAT3 upF3{};
		XMStoreFloat3(&upF3, up);
		setUp(upF3);
	}
}