#include "GameMap.h"
#include <3D/Billboard/Billboard.h>
#include <Util/YamlLoader.h>
#include <Util/Util.h>
#include <DirectXMath.h>

#include "GameCamera.h"

using namespace DirectX;

void GameMap::setAABBData(size_t x, size_t y, const DirectX::XMFLOAT3& pos, float scale, const std::string& cellStr)
{
	auto& data = chipData[y][x];

	data.cellStr = cellStr;

	const float harfScale = scale / 2.f;
	const XMFLOAT2 minPos(pos.x - harfScale, pos.y - harfScale);
	const XMFLOAT2 maxPos(pos.x + harfScale, pos.y + harfScale);

	data.aabb.minPos = XMLoadFloat2(&minPos);
	data.aabb.maxPos = XMLoadFloat2(&maxPos);
}

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

	chipData.resize(csvData.size());

	for (size_t y = 0u, yLen = csvData.size(); y < yLen; ++y)
	{
		chipData[y].resize(csvData[y].size());

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

			// 判定作成
			setAABBData(x, y, pos, chipSize, cellStr);
		}
	}

	return false;
}

void GameMap::update()
{
	const auto angle = XMConvertToRadians(-camera->getAngleDeg());
	for (auto& i : billboard)
	{
		i.second->setRotation(angle);
		i.second->update();
	}
}

void GameMap::draw()
{
	for (auto& i : billboard)
	{
		i.second->draw();
	}
}