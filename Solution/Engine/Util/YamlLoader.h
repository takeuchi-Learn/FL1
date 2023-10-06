#pragma once

#include <Yaml.hpp>

#ifndef YAML_LOADER_NO_FUNC_MACRO

// @brief データをYAMLから変数に読み込む関数マクロ
// @note キー文字列と引数に入れる変数名が一致している必要がある
#define LoadYamlData(NODE, VAR) { VAR = NODE[#VAR].As<decltype(VAR)>(); }

// @brief データをYAMLからfloat2変数に読み込む関数マクロ
// @details 中括弧による代入{x, y}が可能な型に限る
// @note キー文字列と引数に入れる変数名が一致している必要がある
#define LoadYamlDataToFloat2(NODE, VAR)\
{\
	auto& varNode = NODE[#VAR];\
	VAR = {\
		varNode["x"].As<float>(),\
		varNode["y"].As<float>()\
	};\
}

// @brief データをYAMLからfloat3変数に読み込む関数マクロ
// @details 中括弧による代入{x, y, z}が可能な型に限る
// @note キー文字列と引数に入れる変数名が一致している必要がある
#define LoadYamlDataToFloat3(NODE, VAR)\
{\
	auto& varNode = NODE[#VAR];\
	VAR = {\
		varNode["x"].As<float>(),\
		varNode["y"].As<float>(),\
		varNode["z"].As<float>()\
	};\
}

// @brief データをYAMLからfloat4変数に読み込む関数マクロ
// @details 中括弧による代入{x, y, z, w}が可能な型に限る
// @note キー文字列と引数に入れる変数名が一致している必要がある
#define LoadYamlDataToFloat4(NODE, VAR)\
{\
	auto& varNode = NODE[#VAR];\
	VAR = {\
		varNode["x"].As<float>(),\
		varNode["y"].As<float>(),\
		varNode["z"].As<float>(),\
		varNode["w"].As<float>()\
	};\
}

#endif // !YAML_LOADER_NO_FUNC_MACRO

class YamlLoader
{
public:
	/// @brief YAMLデータをファイルから読み込む
	/// @param node 読み込んだデータを格納するノード
	/// @param filePath 読み込むYAMLファイルのパス
	/// @return エラーがあるかどうか
	static inline bool LoadYamlFile(Yaml::Node& node, const std::string& filePath)
	{
		try
		{
			Yaml::Parse(node, filePath.c_str());
		} catch (...)
		{
			return true;
		}
		return false;
	}
};
