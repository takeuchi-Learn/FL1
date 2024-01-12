#include "Looper.h"

#include <DirectXMath.h>
#include "3D/Obj/Object3d.h"
#include "Sound/SoundBase.h"
#include "Input/Input.h"
#include "SceneManager.h"
using namespace DirectX;

Looper::Looper()
{
	Object3d::staticInit();

	// Soundのクラスより先に消えないようにここで生成しておく
	SoundBase::getInstange();

	pushImGuiCol();
}

void Looper::pushImGuiCol()
{
	constexpr float whiteVal = float(0xf8) / 256.f;
	constexpr float blackVal = float(0x03) / 256.f;

	constexpr XMFLOAT3 guiCol = XMFLOAT3(blackVal, blackVal, blackVal);
	constexpr float guiColDarkVel = 0.25f;
	constexpr XMFLOAT3 guiColTitleBg = XMFLOAT3(guiCol.x * guiColDarkVel,
												guiCol.y * guiColDarkVel,
												guiCol.z * guiColDarkVel);
	constexpr float guiBgVel = 0.5f;
	constexpr XMFLOAT3 guiBg = XMFLOAT3(whiteVal * guiBgVel,
										whiteVal * guiBgVel,
										whiteVal * guiBgVel);

	constexpr XMFLOAT3 textCol = guiCol;

	constexpr float guiAlpha = 0.25f;
	constexpr float guiTitleAlpha = guiAlpha * 2.f;

	// タイトルバーの色
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TitleBg, ImVec4(guiColTitleBg.x,
															  guiColTitleBg.y,
															  guiColTitleBg.z,
															  guiTitleAlpha));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TitleBgActive, ImVec4(guiCol.x,
																	guiCol.y,
																	guiCol.z,
																	guiTitleAlpha));

	// 背景の色
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, ImVec4(guiBg.x,
															   guiBg.y,
															   guiBg.z,
															   guiAlpha));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ScrollbarBg, ImVec4(guiBg.x,
																  guiBg.y,
																  guiBg.z,
																  guiAlpha));

	// その他の色
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(textCol.x,
														   textCol.y,
														   textCol.z,
														   1.f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Border, ImVec4(guiCol.x,
															 guiCol.y,
															 guiCol.z,
															 0.5f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ScrollbarGrab, ImVec4(guiCol.x,
																	guiCol.y,
																	guiCol.z,
																	0.5f));
}

void Looper::popImGuiCol()
{
	ImGui::PopStyleColor(7);
}

Looper::~Looper()
{
	popImGuiCol();
}

void Looper::loopUpdate()
{
	// 入力情報の更新
	Input::getInstance()->update();

	// --------------------
	// シーンマネージャーの更新
	// --------------------
	SceneManager::getInstange()->update();
}

void Looper::loopDraw()
{
	// --------------------
	// シーンマネージャーの描画
	// --------------------
	constexpr DirectX::XMFLOAT3 clearColor = { 0.f, 0.f, 0.f };	//黒色

	// ポストエフェクト内の描画
	PostEffect::ins()->startDrawScene(DX12Base::ins());
	SceneManager::ins()->drawObj3d();
	PostEffect::ins()->endDrawScene(DX12Base::ins());

	// 全体の描画
	DX12Base::ins()->startDraw();
	DX12Base::ins()->clearBuffer(clearColor);
	DX12Base::ins()->startImGui();

	PostEffect::ins()->draw(DX12Base::ins());

	SceneManager::ins()->drawFrontSprite();

	DX12Base::ins()->endImGui();
	DX12Base::ins()->endDraw();

	SceneManager::ins()->draw_afterImGui();
}

bool Looper::loop()
{
	loopUpdate();
	loopDraw();

	return exitFlag;
}