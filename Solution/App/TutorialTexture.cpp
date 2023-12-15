#include "TutorialTexture.h"

#include<DirectXMath.h>
#include<3D/Billboard/Billboard.h>
#include<GameCamera.h>

using namespace DirectX;

namespace
{
	const wchar_t* texPath = L"Resources/Map/Tex/blockobj.png";
};

TutorialTexture::TutorialTexture(GameCamera* camera)
	:gameObj(std::make_unique<Billboard>(texPath, camera))
	,camera(camera)
{
	// 数値は仮設定
	gameObj->add(XMFLOAT3(300, -700,0), 300.f);

}

void TutorialTexture::update()
{
	gameObj->update(XMConvertToRadians(-camera->getAngleDeg()));
}

void TutorialTexture::draw()
{
	gameObj->draw();
}
