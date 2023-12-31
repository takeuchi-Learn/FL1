﻿#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include "Mesh.h"
#include "System/DX12Base.h"

class ObjModel
{
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

	// --------------------
	// static
	// --------------------
public:

private:
	static DX12Base* dxBase;
	// デスクリプタサイズ
	static UINT descriptorHandleIncrementSize;

	// --------------------
	// メンバ変数
	// --------------------
private:
	std::string name;
	std::vector<std::unique_ptr<Mesh>> meshes;
	std::unordered_map<std::string, std::unique_ptr<Material>> materials;
	// デフォルトマテリアル
	Material* defaultMaterial = nullptr;
	// デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeap;
	DirectX::XMFLOAT2 texTilling{ 1, 1 };
	DirectX::XMFLOAT2 shiftUv{ 0, 0 };
	DirectX::XMFLOAT3 ambient{ 0.3f, 0.3f, 0.3f };
	DirectX::XMFLOAT3 diffuse{};
	DirectX::XMFLOAT3 specular{ 0.5f, 0.5f, 0.5f };
	bool materialDirty = false;
	bool pbr = false;

	/// @brief マテリアル読み込み
	void loadMaterial(const std::string& directoryPath, const std::string& filename);

	/// @brief デスクリプタヒープの生成
	void createDescriptorHeap();

	/// @brief テクスチャ読み込み
	void loadTextures(const std::string& dirPath, UINT texNum);

public:
	// ----------
	// static
	// ----------

	/// @brief メッシュ情報を取得
	/// @return メッシュ情報
	inline const auto& getMesh() const { return meshes; }

	/// @brief マテリアつ情報を取得
	inline const auto& getMaterial() const { return materials; }

	inline const auto getOneMaterialItr() const { return materials.begin(); }

	/// @brief テクスチャのタイリングを設定
	/// @param tillingNum タイリングの数
	inline void setTexTilling(const DirectX::XMFLOAT2& tillingNum) { texTilling = tillingNum; materialDirty = true; }
	inline const DirectX::XMFLOAT2& getTexTilling() const { return texTilling; }

	inline void setShivtUv(const DirectX::XMFLOAT2& shiftNum) { shiftUv = shiftNum; materialDirty = true; }
	inline const DirectX::XMFLOAT2& getShiftUv() const { return shiftUv; }

	inline void setAmbient(const DirectX::XMFLOAT3& ambient) { this->ambient = ambient; materialDirty = true; }
	inline void setDiffuse(const DirectX::XMFLOAT3& diffuse) { this->diffuse = diffuse; materialDirty = true; }
	inline void setSpecular(const DirectX::XMFLOAT3& specular) { this->specular = specular; materialDirty = true; }

	/// @param dirPath : objファイルのある場所のパス(例 : Resources/player/)
	/// @param objModelName : objファイルのファイル名(拡張子なし。例 : player.obj -> player)
	ObjModel(const std::string& dirPath, const std::string& objModelName, UINT texNum = 0u, bool smoothing = false, bool pbr = false);
	~ObjModel();

	/// @brief 初期化
	/// @param dirPath モデルファイルのあるファイルパス
	/// @param modelname モデル名
	/// @param texNum テクスチャの識別番号
	/// @param smoothing スムースシェーディングにするかどうか
	void init(const std::string& dirPath, const std::string& modelname, UINT texNum = 0u, bool smoothing = false);

	/// @brief 描画
	/// @param cmdLst 命令発行先コマンドリスト
	void draw(ID3D12GraphicsCommandList* cmdList);
};
