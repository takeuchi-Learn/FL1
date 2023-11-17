#include "GameMap.h"
#include <3D/Billboard/Billboard.h>
#include <Util/YamlLoader.h>
#include <Util/Util.h>
#include <DirectXMath.h>

#include "GameCamera.h"

using namespace DirectX;

void GameMap::setAABBData(size_t x, size_t y, const DirectX::XMFLOAT3& pos, float scale)
{
	const float harfScale = scale / 2;
	XMFLOAT2 minPos(pos.x - harfScale, pos.y - harfScale);
	XMFLOAT2 maxPos(pos.x + harfScale, pos.y + harfScale);
	mapAABBs[y][x].minPos = XMLoadFloat2(&minPos);
	mapAABBs[y][x].maxPos = XMLoadFloat2(&maxPos);

	//XMFLOAT2 minPos(pos.x, pos.y );
	//XMFLOAT2 maxPos(pos.x + scale, pos.y + scale);
	//mapAABBs[y][x].minPos = XMLoadFloat2(&minPos);
	//mapAABBs[y][x].maxPos = XMLoadFloat2(&maxPos);
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

	mapAABBs.resize(csvData.size());


	for (size_t y = 0u, yLen = csvData.size(); y < yLen; ++y)
	{
		mapAABBs[y].resize(csvData[y].size());

		for (size_t x = 0u, xLen = csvData[y].size(); x < xLen; ++x)
		{
			const auto& cellStr = csvData[y][x];

			constexpr int base = 10;
			uint8_t n = MAPCHIP_UNDEF;
			const auto ret = std::from_chars(std::to_address(cellStr.begin()),
											 std::to_address(cellStr.end()),
											 n,
											 base);

			// 未定義の値なら
			if (ret.ec == std::errc::invalid_argument ||
				ret.ec == std::errc::result_out_of_range ||
				n == MAPCHIP_UNDEF ||
				n >= MAPCHIP_ALLNUM)
			{
				assert(0);
				return true;
			}

			// 道なら何も表示しない
			if (n == MAPCHIP_ROAD)
			{
				continue;
			}

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
			const auto addRet = billboard.try_emplace(MAPCHIP_DATA(n), std::make_unique<Billboard>(wTexPath.c_str(), camera));
			auto& data = addRet.first->second;
			data->setCamera(camera);
			
			// 新たに挿入されたら addRet.second == true

			constexpr auto scale = float(WinAPI::window_height) / 10.f;
			const auto pos = XMFLOAT3(float(x) * scale,
									  -float(y) * scale,
									  0);

			data->add(pos, scale);

			// 道なら判定作成しない
			if (n != MAPCHIP_ROAD
				&& n != MAPCHIP_GOAL)
			{
				// 判定作成
				setAABBData(x, y, pos, scale);
			}
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

float GameMap::getGameoverPos()const
{
	return mapAABBs[mapAABBs.size() - 1][0].minPos.m128_f32[1];
}
