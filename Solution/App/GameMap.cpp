#include "GameMap.h"
#include <3D/Billboard/Billboard.h>
#include <Util/YamlLoader.h>
#include <Util/Util.h>
#include <DirectXMath.h>
#include <Object/Goal.h>
#include <Object/ColorCone.h>
#include "GameCamera.h"
#include <sstream>

using namespace DirectX;

namespace
{
	constexpr char* coneStr = "cone";
	constexpr char* goalStr = "goal";

	// オブジェクト一覧
	constexpr std::array<char*, 2> objectNames =
	{
		coneStr,
		goalStr,
	};

	template <std::integral Ty, int base = 36>
	std::from_chars_result fromStr(std::string_view str, Ty& ret)
	{
		return std::from_chars(std::to_address(str.begin()),
							   std::to_address(str.end()),
							   ret, base);
	}

	template <std::floating_point Ty>
	std::from_chars_result fromStr(std::string_view str, Ty& ret)
	{
		return std::from_chars(std::to_address(str.begin()),
							   std::to_address(str.end()),
							   ret);
	}
}

GameMap::Collider::Collider(const DirectX::XMFLOAT2& minPos,
							const DirectX::XMFLOAT2& maxPos,
							uint8_t collisionBitFlag)
	: aabb({ XMLoadFloat2(&minPos), XMLoadFloat2(&maxPos) }),
	collisionDirectionBitFlag(collisionBitFlag)
{}

void GameMap::setAABBData(size_t x, size_t y,
						  const DirectX::XMFLOAT3& pos,
						  float scale,
						  uint8_t collisionBitFlag)
{
	const float harfScale = scale / 2.f;
	const XMFLOAT2 minPos(pos.x - harfScale, pos.y - harfScale);
	const XMFLOAT2 maxPos(pos.x + harfScale, pos.y + harfScale);

	mapAABBs.emplace_back(minPos, maxPos, collisionBitFlag);
}

GameMap::GameMap(GameCamera* camera)
	: camera(camera)
{}

void GameMap::loadStageObjList(const std::string& csvStr, float scale, const std::function<void(const DirectX::XMFLOAT2&)> insertFunc)
{
	auto csv = Util::loadCsvFromString(csvStr);

	float posBuf[2]{};

	for (auto& y : csv)
	{
		const auto yLen = y.size();
		if (yLen < 2) { continue; }

		for (uint8_t i = 0ui8; i < 2ui8; ++i)
		{
			fromStr(y[i], posBuf[i]);
			posBuf[i] *= scale;
		}
		posBuf[1] = -posBuf[1];

		insertFunc(XMFLOAT2(posBuf));
	}
}

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

	if (auto& collisionNode = root["collisionBitFlag"];
		collisionNode.Size() > 0)
	{
		for (auto i = collisionNode.Begin(); i != collisionNode.End(); i++)
		{
			const auto& key = (*i).first;
			const auto& node = (*i).second;

			const auto str = node.As<std::string>("1111");

			constexpr int base = 2;
			fromStr<uint8_t, base>(str, collisionDataList[key]);
		}
	}

	const auto texFolderPath = root["texFolderPath"].As<std::string>("");
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

			constexpr int base = 36;
			uint8_t n = UINT8_MAX;
			const auto ret = fromStr<uint8_t, base>(cellStr, n);

			// 未定義の値なら
			if (ret.ec == std::errc::invalid_argument ||
				ret.ec == std::errc::result_out_of_range)
			{
				assert(0);
				return true;
			}

			const auto pos = XMFLOAT3(float(x) * scale,
									  -float(y) * scale,
									  0.f);

			{
				// コリジョンデータが未指定なら全方向判定を取る
				uint8_t currentCollisionData = 0b1111ui8;

				// コリジョンデータがあればそれを使う
				if (auto it = collisionDataList.find(cellStr);
					it != collisionDataList.end())
				{
					currentCollisionData = it->second;
				}

				// コリジョンデータに一方向でも判定が有ればコライダーを追加
				if (const uint8_t flags = currentCollisionData & 0b1111ui8;
					flags)
				{
					setAABBData(x, y, pos, scale, flags);
				}
			}

			// パスをstringからwstringに変換
			try
			{
				constexpr const char defStr[] = "/INVISIBLE/";
				std::string texPath = texFileNameNode[cellStr].As<std::string>(defStr);
				if (texPath == defStr || texPath == "") { continue; }

				// string型のパス
				texPath = texFolderPath + texPath;

				// 長さを取得
				const auto len = MultiByteToWideChar(CP_ACP, 0, texPath.c_str(), -1, (wchar_t*)NULL, 0);
				std::vector<wchar_t> buf(len);

				//string -> wstring
				MultiByteToWideChar(CP_ACP, 0, texPath.c_str(), -1, buf.data(), len);
				std::wstring wTexPath = std::wstring(buf.data(), buf.data() + len - 1);

				// ここで "billboard[MAPCHIP_DATA(n)];" 要素を追加する
				// YAML内の画像ファイルパスを反映させる
				const auto addRet = billboard.try_emplace(n, nullptr);
				auto& data = addRet.first->second;
				// 新たに挿入されたら addRet.second == true
				if (addRet.second) { data = std::make_unique<Billboard>(wTexPath.c_str(), camera); }
				data->setCamera(camera);
				data->add(pos, scale);
			} catch (...)
			{
				continue;
			}
		}
	}

	mapAABBs.shrink_to_fit();

	// オブジェクト座標読み込み
	{
		constexpr const char defStr[] = "NONE";

		auto csvStr = root["goal"].As<std::string>(defStr);
		if (csvStr != defStr)
		{
			loadStageObjList(csvStr, scale,
							 [&](const XMFLOAT2& pos)
							 {
								 constexpr auto texSize = XMFLOAT2(800.f, 300.f);
								 auto& i = goals.emplace_front(std::make_unique<Goal>(camera, pos, texSize));

								 constexpr auto centerPx = XMFLOAT2(249.f, 249.f);
								 constexpr auto centerUv = XMFLOAT2(1.f - centerPx.x / texSize.x, centerPx.y / texSize.y);
								 constexpr auto center = XMFLOAT2(std::lerp(-1.f, 1.f, centerUv.x), std::lerp(-1.f, 1.f, centerUv.y));
								 i->setCenter(center);
							 });
		}

		csvStr = root["cone"].As<std::string>(defStr);
		if (csvStr != defStr)
		{
			loadStageObjList(csvStr, scale, [&](const XMFLOAT2& pos) { cones.emplace_front(std::make_unique<ColorCone>(camera, pos, scale)); });
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

	for (auto& i : cones)
	{
		i->update();
	}

	for (auto& i : goals)
	{
		i->update();
	}
}

void GameMap::draw()
{
	for (auto& i : billboard)
	{
		i.second->draw();
	}

	for (auto& i : cones)
	{
		i->draw();
	}

	for (auto& i : goals)
	{
		i->draw();
	}
}

float GameMap::calcGameoverPos() const
{
	// ゲームオーバー座標に減算する数値(これでゲームオーバー判定地点を変更できる)
	constexpr float gameOverPosSubNum = 150.f;
	return XMVectorGetY(mapAABBs.back().aabb.minPos) - gameOverPosSubNum;
}