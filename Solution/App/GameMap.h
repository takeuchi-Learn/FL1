#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Billboard;
class GameCamera;

class GameMap
{
public:
	static constexpr float chipSize = 100.f;

private:
	std::unordered_map<std::string, std::unique_ptr<Billboard>> billboard;

	GameCamera* camera = nullptr;

public:
	GameMap(GameCamera* camera);
	~GameMap() = default;

	/// @brief YAMLファイルから読み込み
	/// @param filePath YAMLファイルパス
	/// @return エラーがあるかどうか
	bool loadDataFile(const std::string& filePath);

	void update();
	void draw();
};
