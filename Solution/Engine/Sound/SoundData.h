#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <wrl.h>
#include "Sound.h"

class SoundData final
{
private:
	friend class Sound;

public:
	SoundData() = default;

	// 音声データの解放
	~SoundData();
	// 音声データの読み込み
	SoundData(const char* filename);

	using WAVEFORM = Sound::WAVEFORM;

	SoundData(WAVEFORM waveform, float Hz, float sec = 1ui16);

private:
#pragma region チャンク
	// チャンクヘッダ
	struct ChunkHeader
	{
		char id[4]; // チャンク毎のID
		int32_t size;  // チャンクサイズ
	};

	// RIFFヘッダチャンク
	struct RiffHeader
	{
		ChunkHeader chunk;   // "RIFF"
		char type[4]; // "WAVE"
	};

	// FMTチャンク
	struct FormatChunk
	{
		ChunkHeader chunk; // "fmt "
		WAVEFORMATEX fmt; // 波形フォーマット
	};
#pragma endregion

	// --------------------
	// メンバ変数
	// --------------------
private:
	//波形フォーマット
	WAVEFORMATEX wfex{};
	//バッファの先頭アドレス
	BYTE* pBuffer = nullptr;
	//バッファのサイズ
	unsigned int bufferSize = 0u;

	IXAudio2SourceVoice* pSourceVoice = nullptr;

	// --------------------
	// メンバ関数
	// --------------------
public:
	inline auto stopVoice()
	{
		return pSourceVoice->Stop();
	}

	inline auto startVoice()
	{
		return pSourceVoice->Start();
	}

	// --------------------
	// static関数
	// --------------------
private:
	static void createSourceVoice(SoundData* soundData);

private:
	// 音声再生停止
	static void SoundStopWave(SoundData* soundData);

	/// @brief 音声再生
	/// @param soundData 再生するサウンドデータオブジェクト
	/// @param loopCount 0で繰り返し無し、XAUDIO2_LOOP_INFINITEで永遠
	/// @param volume 0 ~ 1
	static void SoundPlayWave(SoundData* soundData,
							  UINT32 loopCount = 0, float volume = 0.2);

	//再生状態の確認
	static bool checkPlaySound(SoundData* soundData);
};
