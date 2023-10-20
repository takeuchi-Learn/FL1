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
	auto& csv = root["CSV"].As<std::string>("NONE");

	const auto csvData = Util::loadCsvFromString(csv);

	for (size_t y = 0u, yLen = csvData.size(); y < yLen; ++y)
	{
		for (size_t x = 0u, xLen = csvData[y].size(); x < xLen; ++x)
		{
			constexpr int base = 10;
			uint8_t n = MAPCHIP_UNDEF;
			const auto ret = std::from_chars(std::to_address(csvData[y][x].begin()),
											 std::to_address(csvData[y][x].end()),
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

			// ここで "billboard[MAP_CHIP_DATA(n)];" 要素を追加する
			// todo YAML内の画像ファイルパスを反映させる
			const auto addRet = billboard.try_emplace(MAP_CHIP_DATA(n), std::make_unique<Billboard>(L"Resources/judgeRange.png", camera));


			auto& data = addRet.first->second;
			data->setCamera(camera);

			// 新たに挿入されたら
			if (addRet.second)
			{

			}

			constexpr auto scale = float(WinAPI::window_height) / 10.f;

			const auto pos = XMFLOAT3(float(x) * scale - (scale * float(xLen) / 2.f),
									  -float(y) * scale + (scale * float(yLen) / 2.f),
									  0);

			data->add(pos, scale);
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
