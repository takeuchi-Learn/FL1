#pragma once

#include<unordered_map>

// コーン記録クラス
// ステージセレクトでも数を表示したりクリアシーンで倒した数更新した時に今までの記録出すかもしれないので一応シングルトンに
class ConeRecorder
{
private:
	/// @brief ステージ番号,カウント
	std::unordered_map<uint16_t, uint16_t> records;

private:
	ConeRecorder() = default;
	virtual ~ConeRecorder() = default;
	ConeRecorder(ConeRecorder& c) = delete;
	ConeRecorder operator=(ConeRecorder& c) = delete;

public:
	inline static ConeRecorder* ins()
	{
		static ConeRecorder counter{};
		return &counter;
	}

	/// @brief クリアした時に自己ベストだったら記録登録する関数
	/// @return エラーでtrue
	bool registration(const uint16_t stageNum, const uint16_t coneCount);

	const uint16_t getRecord(const uint16_t stageNum) { return records.at(stageNum); }
	const auto& getRecords()const { return records; }
};
