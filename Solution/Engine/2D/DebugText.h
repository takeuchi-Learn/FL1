#pragma once

#include <string>
#include <array>
#include <DirectXMath.h>
#include "Sprite.h"

class DebugText
{
public: // 定数の宣言
	static constexpr uint32_t maxCharCount = 256;    // 最大文字数
	// 画像サイズに合わせて変えたい(fontWidth, fontHeight)
	static constexpr uint32_t fontWidth = 9 * 2;         // フォント画像内1文字分の横幅
	static constexpr uint32_t fontHeight = 18 * 2;       // フォント画像内1文字分の縦幅
	static constexpr uint32_t fontLineCount = 14;    // フォント画像内1行分の文字数

public:
	uint8_t tabSize = 4;	// 初期値はSPACE4つ分

private:
	SpriteBase* spriteBase = nullptr;

public:
	DebugText(uint32_t texNum, SpriteBase* spriteCommon, uint8_t tabSize = 4);

	// ￥n : X座標をして位置に戻し、Y座標を文字の高さ分加算する
	// ￥t : tabSize文字分右にずらす
	void Print(const std::string& text,
			   const float x, const float y, const float scale = 1.f,
			   DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1, 1, 1, 1));

	// 内部でvsnprintfを使用
	// @return vsnprintfの戻り値
	int formatPrint(const float x, const float y, const float scale, DirectX::XMFLOAT4 color, const char* fmt, ...);

	void DrawAll();

private: // メンバ変数
	// スプライトデータの配列
	std::array<Sprite, maxCharCount> sprites;
	// スプライトデータ配列の添え字番号
	uint32_t spriteIndex = 0;
};