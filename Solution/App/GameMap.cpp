#include "GameMap.h"
#include <3D/Billboard/Billboard.h>
#include <Util/YamlLoader.h>
#include <Util/Util.h>
#include <DirectXMath.h>

#include "GameCamera.h"

using namespace DirectX;

GameMap::GameMap(GameCamera* camera) :
	camera(camera)
{}

bool GameMap::loadDataFile(const std::string& filePath)
{
	Yaml::Node root{};
	if (YamlLoader::LoadYamlFile(root, filePath))
	{
		return true;
	}
	const auto& texFolderPath = root["texFolderPath"].As<std::string>("DEF_VALUE");
	auto& texFileNameNode = root["texFileName"];

	auto& csv = root["csv"].As<std::string>("NONE");

	const auto csvData = Util::loadCsvFromString(csv);

	for (size_t y = 0u, yLen = csvData.size(); y < yLen; ++y)
	{
		for (size_t x = 0u, xLen = csvData[y].size(); x < xLen; ++x)
		{
			const auto& cellStr = csvData[y][x];

			std::wstring wTexPath{};

			// パスをstringからwstringに変換
			try
			{
				// string型のパス
				const std::string texPath = texFolderPath + texFileNameNode[cellStr].As<std::string>();
				// 長さを取得
				const auto len = MultiByteToWideChar(CP_ACP, 0, texPath.c_str(), -1, (wchar_t*)NULL, 0);
				std::vector<wchar_t> buf(len);

				//string -> wstring
				MultiByteToWideChar(CP_ACP, 0, texPath.c_str(), -1, buf.data(), len);
				wTexPath = std::wstring(buf.data(), buf.data() + len - 1);
			} catch (...)
			{
				assert(0);
				return true;
			}

			// ここで "billboard[MAPCHIP_DATA(n)];" 要素を追加する
			// YAML内の画像ファイルパスを反映させる
			const auto addRet = billboard.try_emplace(cellStr, nullptr);
			auto& data = addRet.first->second;
			// 新たに挿入されたら要素を構築する
			if (addRet.second)
			{
				data = std::make_unique<Billboard>(wTexPath.c_str(), camera);
			}
			data->setCamera(camera);

			// 新たに挿入されたら addRet.second == true

			const auto pos = XMFLOAT3(float(x) * chipSize,
									  -float(y) * chipSize,
									  0);

			data->add(pos, chipSize + 1.f);
		}
	}

	return false;
}

void GameMap::update()
{
	for (auto& i : billboard)
	{
		i.second->update(XMConvertToRadians(-camera->getAngleDeg()));
	}
}

void GameMap::draw()
{
	for (auto& i : billboard)
	{
		i.second->draw();
	}
}
