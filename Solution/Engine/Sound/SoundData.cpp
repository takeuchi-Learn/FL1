#include "SoundData.h"

#include "SoundBase.h"

#include <fstream>
#include <cassert>
#include <functional>

namespace
{
	inline float sin01(float i, float length)
	{
		constexpr float PI2 = static_cast<float>(2.0 * 3.1415926535897932384626433832795);
		return std::sin(PI2 * i / length);
	}

	// のこぎり波
	float sawtoothSound(float i, float length)
	{
		return (std::fmod(i / length, 1.f) + 1.f) / 2.f;
	}

	// sin波を[0 ~ 1]にしたもの
	float sinSound(float i, float length)
	{
		return (sin01(i, length) + 1.f) / 2.f;
	}

	// 矩形波
	float squareSound(float i, float length)
	{
		return sin01(i, length) > 0.f ? 1.f : 0.f;
	}
}

SoundData::~SoundData()
{
	SoundStopWave(this);

	if (this->pSourceVoice != nullptr)
	{
		this->pSourceVoice->DestroyVoice();
	}

	//バッファのメモリを解放
	delete[] this->pBuffer;

	this->pBuffer = nullptr;
	this->bufferSize = 0u;
	this->wfex = {};
}

SoundData::SoundData(const char* filename)
{
	//1.ファイルオープン
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	//ファイルオープン失敗を検出する
	assert(file.is_open());

	//2.wavデータ読み込み
	//RIFFヘッダーの読み込み
	RiffHeader riff{};
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}
	//タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0)
	{
		assert(0);
	}
	//Formatチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0)
	{
		assert(0);
	}
	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);
	//Dataチャンクの読み込み
	ChunkHeader data{};
	file.read((char*)&data, sizeof(data));
	//JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0)
	{
		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		//再読み込み
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0)
	{
		assert(0);
	}
	//Dataチャンクデータの一部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//3.ファイルクローズ
	//Waveファイルを閉じる
	file.close();

	//4.読み込んだ音声データを格納

	this->wfex = format.fmt;
	this->pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	this->bufferSize = data.size;

	createSourceVoice(this);
}

SoundData::SoundData(WAVEFORM waveform, float Hz, float sec) :
	SoundData()
{
	constexpr DWORD samplingRate = 44100;
	constexpr WORD bitDepth = 8;
	constexpr WORD channelNum = 1;

	constexpr WORD blockSizeData = bitDepth / 8 * channelNum;
	constexpr DWORD bufEstimation = samplingRate * blockSizeData;
	constexpr float bufEstimationF = static_cast<float>(bufEstimation);

	WAVEFORMATEX& format = this->wfex;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = channelNum;			// チャンネル数(1: モノラル、2: ステレオ)
	format.wBitsPerSample = bitDepth;		// 1サンプルのビット数
	format.nSamplesPerSec = samplingRate;	// サンプリングレート
	format.nBlockAlign = blockSizeData;
	format.nAvgBytesPerSec = bufEstimation;

	// sec秒分のバッファ
	const size_t dataSize = static_cast<size_t>(bufEstimationF * sec);
	BYTE* data = new BYTE[dataSize];

	{
		const float length = (float)format.nSamplesPerSec / Hz;

		float(*soundFunc)(float i, float length) = sawtoothSound;

		switch (waveform)
		{
		case SoundData::WAVEFORM::SIN:
			soundFunc = sinSound;
			break;
		case SoundData::WAVEFORM::SQUARE:
			soundFunc = squareSound;
			break;
		case SoundData::WAVEFORM::SAWTOOTH:
		default:
			soundFunc = sawtoothSound;
			break;
		}

		for (size_t i = 0ui64; i < dataSize; ++i)
		{
			const float s = soundFunc(static_cast<float>(i), length);

			// 上の値を[0 ~ 255]にして格納
			data[i] = BYTE(255.f * s);
		}
	}

	createSourceVoice(this);

	bufferSize = (unsigned int)dataSize;
	pBuffer = reinterpret_cast<BYTE*>(data);
}

void SoundData::createSourceVoice(SoundData* soundData)
{
	//波形フォーマットをもとにSourceVoiceの生成
	HRESULT result = SoundBase::getInstange()->xAudio2->CreateSourceVoice(&soundData->pSourceVoice, &soundData->wfex);
	assert(SUCCEEDED(result));
}

void SoundData::SoundStopWave(SoundData* soundData)
{
	HRESULT result = soundData->pSourceVoice->Stop();
	assert(SUCCEEDED(result));

	result = soundData->pSourceVoice->FlushSourceBuffers();
	assert(SUCCEEDED(result));

	/*XAUDIO2_BUFFER buf{};
	result = soundData.pSourceVoice->SubmitSourceBuffer(&buf);*/
}

void SoundData::SoundPlayWave(SoundData* soundData, UINT32 loopCount, float volume)
{
	//波形フォーマットをもとにSourceVoiceの生成
	createSourceVoice(soundData);

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData->pBuffer;
	buf.AudioBytes = soundData->bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	if (loopCount > 0)
	{
		buf.LoopCount = loopCount;
	} else
	{
		buf.LoopCount = 0u;
		buf.LoopBegin = 0u;
		buf.LoopLength = 0u;
	}

	//波形データの再生
	HRESULT result = soundData->pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = soundData->pSourceVoice->SetVolume(volume);
	assert(SUCCEEDED(result));

	result = soundData->pSourceVoice->Start();
	assert(SUCCEEDED(result));
}

bool SoundData::checkPlaySound(SoundData* soundData)
{
	if (soundData == nullptr ||
		soundData->pSourceVoice == nullptr) return false;

	XAUDIO2_VOICE_STATE tmp{};
	soundData->pSourceVoice->GetState(&tmp);
	if (tmp.BuffersQueued == 0U)
	{
		return false;
	}
	return true;
}