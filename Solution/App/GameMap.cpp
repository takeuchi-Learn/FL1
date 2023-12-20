#include "GameMap.h"
#include <3D/Billboard/Billboard.h>
#include <Util/YamlLoader.h>
#include <Util/Util.h>
#include <DirectXMath.h>

#include "Object/Goal.h"
#include "Object/ColorCone.h"
#include "GameCamera.h"

#include <sstream>

using namespace DirectX;

namespace
{
	constexpr char* coneStr = "cone";
	constexpr char* goalStr = "goal";

	// オブジェクト一覧
	const std::vector<char*> objectNames =
	{
		coneStr,
		goalStr,
	};
}

void GameMap::setAABBData(size_t x, size_t y, const DirectX::XMFLOAT3& pos, float scale)
{
	const float harfScale = scale / 2.f;
	const XMFLOAT2 minPos(pos.x - harfScale, pos.y - harfScale);
	const XMFLOAT2 maxPos(pos.x + harfScale, pos.y + harfScale);

	mapAABBs.emplace_back(XMLoadFloat2(&minPos),
						  XMLoadFloat2(&maxPos));
}

GameMap::GameMap(GameCamera* camera)
	: camera(camera)
{}

bool GameMap::loadDataFile(const std::string& filePath, DirectX::XMFLOAT2* startPosBuf)
{
	Yaml::Node root{};
	if (YamlLoader::LoadYamlFile(root, filePath))
	{
		return true;
	}

	if (startPosBuf)
	{
		XMFLOAT2& startPos = *startPosBuf;
		LoadYamlDataToFloat2(root, startPos);
	}

	const auto& texFolderPath = root["texFolderPath"].As<std::string>("DEF_VALUE");
	auto& texFileNameNode = root["texFileName"];

	auto& csv = root["csv"].As<std::string>("NONE");

	const auto csvData = Util::loadCsvFromString(csv);

	mapAABBs.reserve(csvData.size());
	mapSizeX = static_cast<unsigned int>(csvData[0].size());
	mapSizeY = static_cast<unsigned int>(csvData.size());

	constexpr auto scale = float(WinAPI::window_height) / 10.f;
	for (size_t y = 0u, yLen = csvData.size(); y < yLen; ++y)
	{
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

			const auto pos = XMFLOAT3(float(x) * scale,
									  -float(y) * scale,
									  0);

			// 通れる道ならcontinueする
			// 一旦GOALもcontinue
			if (n == GameMap::MAPCHIP_ROAD || n == GameMap::MAPCHIP_GOAL)continue;

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

			data->add(pos, scale);

			// 判定作成
			setAABBData(x, y, pos, scale);
		}
	}

	mapAABBs.shrink_to_fit();

	// オブジェクト座標読み込み
	loadStageObject(root, scale);

	return false;
}

void GameMap::loadStageObject(Yaml::Node& node, const float scale)
{
	std::unordered_map<std::string, std::vector<XMFLOAT2>>stageObjectPos;
	// 全オブジェクトの座標をu_mapに格納
	for (auto& name : objectNames)
	{
		const std::string posString = node[name].As<std::string>("NONE");
		if (posString == "NONE") { continue; }
		const auto posStringData = Util::loadCsvFromString(posString);

		std::vector<XMFLOAT2> objectPos(posStringData.size());
		loadStageObjectPosition(posStringData, objectPos);

		// スケールをかける
		for (auto& pos : objectPos)
		{
			pos.x *= scale;
			pos.y *= -scale;
		}

		stageObjectPos.emplace(name, objectPos);
	}

	// オブジェクト配置
	setStageObjects(stageObjectPos, scale);
}

void GameMap::loadStageObjectPosition(const Util::CSVType& posCSV, std::vector<XMFLOAT2>& output)
{
	for (size_t y = 0u, yLen = posCSV.size(); y < yLen; ++y)
	{
		std::from_chars(std::to_address(posCSV[y][0].begin()),
						std::to_address(posCSV[y][0].end()),
						output[y].x);
		std::from_chars(std::to_address(posCSV[y][1].begin()),
						std::to_address(posCSV[y][1].end()),
						output[y].y);
	}
}

void GameMap::setStageObjects(const std::unordered_map<std::string, std::vector<XMFLOAT2>>& stageObjectPos, float scale)
{
	// リサイズ
	size_t sizeNum = 0;
	for (auto& posUMap : stageObjectPos)
	{
		for (auto& pos : posUMap.second)
		{
			++sizeNum;
		}
	}
	stageObjects.resize(sizeNum);

	size_t current = 0;
	for (auto& posUMap : stageObjectPos)
	{
		for (auto& pos : posUMap.second)
		{
			if (posUMap.first == coneStr)
			{
				stageObjects[current] = std::make_unique<ColorCone>(camera, pos, scale);
				++coneMax;
			}
			if (posUMap.first == goalStr)
			{
				stageObjects[current] = std::make_unique<Goal>(camera, pos, scale);
			}
			++current;
		}
	}
}

void GameMap::update()
{
	for (auto& i : billboard)
	{
		i.second->update(XMConvertToRadians(-camera->getAngleDeg()));
	}

	for (auto& obj : stageObjects)
	{
		obj->update();
	}
}

void GameMap::draw()
{
	for (auto& i : billboard)
	{
		i.second->draw();
	}
	for (auto& obj : stageObjects)
	{
		obj->draw();
	}
}

float GameMap::calcGameoverPos() const
{
	// ゲームオーバー座標に減算する数値(これでゲームオーバー判定地点を変更できる)
	constexpr float gameOverPosSubNum = 150.f;
	return XMVectorGetY(mapAABBs.back().minPos) - gameOverPosSubNum;
}