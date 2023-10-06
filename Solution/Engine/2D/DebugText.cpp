#include "DebugText.h"
#include <DirectXMath.h>

DebugText::DebugText(uint32_t texNum, SpriteBase* spriteCommon, uint8_t tabSize) :
	tabSize(tabSize),
	spriteBase(spriteCommon)
{
	// 全てのスプライトデータについて
	for (auto& i : sprites)
	{
		i = Sprite((UINT)texNum, spriteCommon, { 0, 0 });
	}
}

void DebugText::Print(const std::string& text,
					  const float x, const float y, const float scale,
					  DirectX::XMFLOAT4 color)
{
	std::string textLocal = text;

	int posNumX = 0, posNumY = 0;

	// 全ての文字について
	for (UINT i = 0, len = (UINT)text.size(); i < len; ++i, ++posNumX)
	{
		// 最大文字数超過
		if (spriteIndex >= maxCharCount)
		{
			break;
		}

		auto drawCol = color;

		if (i < maxCharCount - 1)
		{
			if (strncmp(&textLocal[i], "\n", 1) == 0)
			{
				posNumX = -1;
				++posNumY;
				textLocal[i] = ' ';
				drawCol.w = 0.f;
			}  if (strncmp(&textLocal[i], "\t", 1) == 0)
			{
				posNumX += (int)tabSize - 1;
				textLocal[i] = ' ';
				drawCol.w = 0.f;
			}
		}

		// 1文字取り出す(※ASCIIコードでしか成り立たない)
		const unsigned char& character = textLocal[i];

		// ASCIIコードの2段分飛ばした番号を計算
		int fontIndex = character - 32;
		if (character >= 0x7f)
		{
			fontIndex = 0;
		}

		int fontIndexY = fontIndex / fontLineCount;
		int fontIndexX = fontIndex % fontLineCount;

		// 座標計算
		sprites[spriteIndex].position = { x + fontWidth * scale * posNumX, y + fontHeight * scale * posNumY, 0 };
		sprites[spriteIndex].color = drawCol;
		sprites[spriteIndex].setTexLeftTop({ (float)fontIndexX * fontWidth, (float)fontIndexY * fontHeight });
		sprites[spriteIndex].setTexSize({ fontWidth, fontHeight });
		sprites[spriteIndex].setSize({ fontWidth * scale, fontHeight * scale });
		// 更新
		sprites[spriteIndex].update(spriteBase);

		// 文字を１つ進める
		++spriteIndex;
	}
}

int DebugText::formatPrint(const float x, const float y, const float scale,
						   DirectX::XMFLOAT4 color, const char* fmt, ...)
{
	char outStrChar[maxCharCount]{};

	constexpr size_t bufferCount = size_t(maxCharCount - 1);

	va_list args{};

	va_start(args, fmt);
	const int ret = vsnprintf(outStrChar, bufferCount, fmt, args);

	Print(outStrChar, x, y, scale, color);
	va_end(args);

	return ret;
}

// まとめて描画
void DebugText::DrawAll()
{
	// 全ての文字のスプライトについて
	for (UINT i = 0; i < (UINT)spriteIndex; ++i)
	{
		// スプライト描画
		sprites[i].draw(DX12Base::ins()->getCmdList(), spriteBase, DX12Base::ins()->getDev());
	}

	spriteIndex = 0;
}