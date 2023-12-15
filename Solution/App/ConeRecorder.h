#pragma once

#include<unordered_map>

// コーン記録クラス
// ステージセレクトでも数を表示したりクリアシーンで倒した数更新した時に今までの記録出すかもしれないので一応シングルトンに
class ConeRecorder
{
private:
	// 記録<ステージ番号,カウント>
	std::unordered_map<unsigned short, unsigned short>records;
	
private:
	ConeRecorder(){}
	~ConeRecorder(){}
public:
	ConeRecorder(ConeRecorder& c) = delete;
	ConeRecorder operator=(ConeRecorder& c) = delete;
	static ConeRecorder* getInstance()
	{
		static ConeRecorder counter{};
		return &counter;
	}

	/// @brief クリアした時に自己ベストだったら記録登録する関数
	void registration(const unsigned short stageNum,const unsigned short coneCount);

	const unsigned shortgetRecord(const unsigned short stageNum) { return records.at(stageNum); }
	const std::unordered_map<unsigned short, unsigned short>& getRecords()const { return records; }

};

