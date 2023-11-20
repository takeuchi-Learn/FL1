#include "ObjModel.h"

#include <string>

#include <d3dx12.h>

#include <DirectXTex.h>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <sstream>
#include <fstream>

#include <algorithm>

using namespace std;

using namespace DirectX;
using namespace Microsoft::WRL;

DX12Base* ObjModel::dxBase = DX12Base::getInstance();
UINT ObjModel::descriptorHandleIncrementSize = 0u;

namespace
{
	constexpr float nearZ = 0.1f, farZ = 1000.f, fog = XM_PI / 3.f;
}

void ObjModel::loadTextures(const std::string& dirPath, UINT texNum)
{
	int textureIndex = 0;
	string directoryPath = dirPath;
	if (dirPath[dirPath.size() - 1] != '/')
	{
		if (dirPath[dirPath.size() - 1] != '\\')
		{
			directoryPath += '/';
		}
	}

	for (auto& m : materials)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), textureIndex, descriptorHandleIncrementSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), textureIndex, descriptorHandleIncrementSize);
		// マテリアルにテクスチャ読み込み
		m.second->loadTexture(directoryPath, texNum, cpuDescHandleSRV, gpuDescHandleSRV);
		++textureIndex;
	}
}

void ObjModel::createDescriptorHeap()
{
	// マテリアルの数
	const size_t count = materials.size();

	// デスクリプタヒープを生成
	if (count > 0u)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
		descHeapDesc.NumDescriptors = (UINT)count; // シェーダーリソースビューの数
		HRESULT result = dxBase->getDev()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));//生成
		assert(SUCCEEDED(result));
	}

	// デスクリプタサイズを取得
	descriptorHandleIncrementSize = dxBase->getDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ObjModel::loadMaterial(const std::string& directoryPath, const std::string& filename)
{
	// ファイルストリーム
	std::ifstream file{};
	// マテリアルファイルを開く
	file.open(directoryPath + filename);
	// ファイルオープン失敗をチェック
	if (file.fail())
	{
		assert(0);
		return;
	}

	std::unique_ptr<Material> material = nullptr;

	// 1行ずつ読み込む
	string line{};
	while (getline(file, line))
	{
		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		// 半角スペース区切りで行の先頭文字列を取得
		string key{};
		getline(line_stream, key, ' ');

		// 先頭のタブ文字は無視する
		if (key[0] == '\t')
		{
			key.erase(key.begin()); // 先頭の文字を削除
		}

		// 先頭文字列がnewmtlならマテリアル名
		if (key == "newmtl")
		{
			// 既にマテリアルがあれば
			if (material)
			{
				// マテリアルをコンテナに登録
				materials.emplace(material->name, std::move(material));
			}

			// 新しいマテリアルを生成
			material = std::make_unique<Material>();
			// マテリアル名読み込み
			line_stream >> material->name;
		}
		// 先頭文字列がKaならアンビエント色
		if (key == "Ka")
		{
			line_stream >> material->ambient.x;
			line_stream >> material->ambient.y;
			line_stream >> material->ambient.z;
		}
		if (pbr)
		{
			// 先頭文字列がNsならroughness
			if (key == "Ns")
			{
				float tmp{};
				line_stream >> tmp;
				tmp /= 1000.f;

				constexpr float gamma = 1.f / 2.2f;
				tmp = std::pow(tmp, gamma);
				material->diffuse = XMFLOAT3(tmp, tmp, tmp);
			}
		} else if (key == "Kd")
		{
			// 先頭文字列がKdならディフューズ色
			line_stream >> material->diffuse.x;
			line_stream >> material->diffuse.y;
			line_stream >> material->diffuse.z;
		}
		// 先頭文字列がKsならスペキュラー色
		if (key == "Ks")
		{
			line_stream >> material->specular.x;
			line_stream >> material->specular.y;
			line_stream >> material->specular.z;
		}
		// 先頭文字列がmap_Kdならテクスチャファイル名
		// テクスチャ画像パス指定を此処の値とbaseDirで行うようにする
		if (key == "map_Kd")
		{
			// テクスチャのファイル名読み込み
			line_stream >> material->texFileName;

			// フルパスからファイル名を取り出す
			size_t pos1 = material->texFileName.rfind('\\');
			if (pos1 != string::npos)
			{
				material->texFileName = material->texFileName.substr(pos1 + 1, material->texFileName.size() - pos1 - 1u);
			}

			pos1 = material->texFileName.rfind('/');
			if (pos1 != string::npos)
			{
				material->texFileName = material->texFileName.substr(pos1 + 1, material->texFileName.size() - pos1 - 1u);
			}
		}
	}
	// ファイルを閉じる
	file.close();

	if (material)
	{
		// マテリアルを登録
		materials.emplace(material->name, std::move(material));
	}
}

ObjModel::ObjModel(const std::string& dirPath, const std::string& objModelName, UINT texNum, bool smoothing, bool pbr) :
	pbr(pbr)
{
	string directoryPath = dirPath;
	if (dirPath[dirPath.size() - 1] != '/')
	{
		if (dirPath[dirPath.size() - 1] != '\\')
		{
			directoryPath += '/';
		}
	}
	init(directoryPath, objModelName, texNum, smoothing);
}

ObjModel::~ObjModel()
{
	meshes.clear();
	materials.clear();
}

void ObjModel::init(const std::string& dirPath, const std::string& modelname, UINT texNum, bool smoothing)
{
	const string filename = modelname + ".obj";
	string directoryPath = dirPath;
	if (dirPath[dirPath.size() - 1] != '/')
	{
		if (dirPath[dirPath.size() - 1] != '\\')
		{
			directoryPath += '/';
		}
	}

	// ファイルストリーム
	std::ifstream file(directoryPath + filename);
	// ファイルオープン失敗をチェック
	assert(!file.fail());

	name = modelname;

	// メッシュ生成
	Mesh* mesh = meshes.emplace_back(std::make_unique<Mesh>()).get();
	int indexCountTex = 0;
	int indexCountNoTex = 0;

	vector<XMFLOAT3> positions;	// 頂点座標
	vector<XMFLOAT3> normals;	// 法線ベクトル
	vector<XMFLOAT2> texcoords;	// テクスチャUV
	// 1行ずつ読み込む
	string line;
	while (getline(file, line))
	{
		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		// 半角スペース区切りで行の先頭文字列を取得
		string key;
		getline(line_stream, key, ' ');

		//マテリアル
		if (key == "mtllib")
		{
			// マテリアルのファイル名読み込み
			string filename;
			line_stream >> filename;
			// マテリアル読み込み
			loadMaterial(directoryPath, filename);
		}
		// 先頭文字列がgならグループの開始
		if (key == "g")
		{
			// カレントメッシュの情報が揃っているなら
			if (mesh->getName().size() > 0 && mesh->getVertexCount() > 0)
			{
				// 頂点法線の平均によるエッジ平滑化
				if (smoothing) mesh->calculateSmoothedVertexNormals();
				// 次のメッシュ生成
				mesh = meshes.emplace_back(std::make_unique<Mesh>()).get();
				indexCountTex = 0;
			}

			// グループ名読み込み
			string groupName;
			line_stream >> groupName;

			// メッシュに名前をセット
			mesh->setName(groupName);
		}
		// 先頭文字列がvなら頂点座標
		if (key == "v")
		{
			// X,Y,Z座標読み込み
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			position.x = -position.x;
			positions.emplace_back(position);
		}
		// 先頭文字列がvtならテクスチャ
		if (key == "vt")
		{
			// U,V成分読み込み
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			// V方向反転
			texcoord.y = 1.f - texcoord.y;
			// テクスチャ座標データに追加
			texcoords.emplace_back(texcoord);
		}
		// 先頭文字列がvnなら法線ベクトル
		if (key == "vn")
		{
			// X,Y,Z成分読み込み
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			normal.x = -normal.x;
			// 法線ベクトルデータに追加
			normals.emplace_back(normal);
		}
		// 先頭文字列がusemtlならマテリアルを割り当てる
		if (key == "usemtl")
		{
			if (mesh->getMaterial() == nullptr)
			{
				// マテリアルの名読み込み
				string materialName;
				line_stream >> materialName;

				// マテリアル名で検索し、マテリアルを割り当てる
				auto itr = materials.find(materialName);
				if (itr != materials.end())
				{
					mesh->setMaterial(itr->second.get());
				}
			}
		}
		// 先頭文字列がfならポリゴン（三角形）
		if (key == "f")
		{
			int faceIndexCount = 0;
			// 半角スペース区切りで行の続きを読み込む
			string index_string;
			while (getline(line_stream, index_string, ' '))
			{
				// 頂点インデックス1個分の文字列をストリームに変換して解析しやすくする
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				// 頂点番号
				index_stream >> indexPosition;

				Material* material = mesh->getMaterial();
				index_stream.seekg(1, ios_base::cur); // スラッシュを飛ばす
				// マテリアル、テクスチャがある場合
				if (material && material->texFileName.size() > 0)
				{
					index_stream >> indexTexcoord;
					index_stream.seekg(1, ios_base::cur); // スラッシュを飛ばす
					index_stream >> indexNormal;
					// 頂点データの追加
					Mesh::VertexPosNormalUv vertex{};
					vertex.pos = positions[size_t(indexPosition - 1)];
					vertex.normal = normals[size_t(indexNormal - 1)];
					vertex.uv = texcoords[size_t(indexTexcoord - 1)];
					mesh->addVertex(vertex);
					// エッジ平滑化用データ追加
					if (smoothing)
					{
						// 座標データ(vキー)の番号と、全て合計した張横店のインデックスをセットで登録する
						mesh->addSmoothData(indexPosition, (unsigned short)mesh->getVertexCount() - 1);
					}
				} else
				{
					char c;
					index_stream >> c;
					// スラッシュ2連続の場合、頂点番号のみ
					if (c == '/')
					{
						// 頂点データの追加
						Mesh::VertexPosNormalUv vertex{};
						vertex.pos = positions[size_t(indexPosition - 1)];
						vertex.normal = { 0, 0, 1 };
						vertex.uv = { 0, 0 };
						mesh->addVertex(vertex);
					} else
					{
						index_stream.seekg(-1, ios_base::cur); // 1文字戻る
						index_stream >> indexTexcoord;
						index_stream.seekg(1, ios_base::cur); // スラッシュを飛ばす
						index_stream >> indexNormal;
						// 頂点データの追加
						Mesh::VertexPosNormalUv vertex{};
						vertex.pos = positions[size_t(indexPosition - 1)];
						vertex.normal = normals[size_t(indexNormal - 1)];
						vertex.uv = { 0, 0 };
						mesh->addVertex(vertex);
					}
				}
				// インデックスデータの追加
				if (faceIndexCount < 3)
				{
					if (faceIndexCount == 2)
					{
						mesh->addIndex(indexCountTex - 2);
						mesh->addIndex(indexCountTex);
						mesh->addIndex(indexCountTex - 1);
					}
				} else
				{
					// 四角形ポリゴンの4点目なので、
					// 四角形の0,1,2,3の内 2,3,0で三角形を構築する。
					// さらに左手系にするために後ろ二つを逆にする。
					mesh->addIndex(indexCountTex - 1);
					mesh->addIndex(indexCountTex - 3);
					mesh->addIndex(indexCountTex);
				}
				++indexCountTex;
				++faceIndexCount;
			}
		}
	}
	file.close();

	// 頂点法線の平均によるエッジ平滑化
	if (smoothing) mesh->calculateSmoothedVertexNormals();

	// メッシュのマテリアルチェック
	for (auto& m : meshes)
	{
		// マテリアルの割り当てがない
		if (m->getMaterial() == nullptr)
		{
			if (defaultMaterial == nullptr)
			{
				constexpr char name[] = "no material";

				// デフォルトマテリアルを生成
				auto& ret = materials.try_emplace(name,
												  std::make_unique<Material>()).first->second;
				defaultMaterial = ret.get();
				defaultMaterial->name = name;
			}
			// デフォルトマテリアルをセット
			m->setMaterial(defaultMaterial);
		}
	}

	// メッシュのバッファ生成
	for (auto& m : meshes)
	{
		m->createBuffers();
	}

	// マテリアルの数値を定数バッファに反映
	for (auto& m : materials)
	{
		m.second->update();
	}

	// デスクリプタヒープ生成
	createDescriptorHeap();

	// テクスチャの読み込み
	loadTextures(directoryPath, texNum);
}

void ObjModel::draw(ID3D12GraphicsCommandList* cmdList)
{
	if (materialDirty)
	{
		// マテリアルの数値を定数バッファに反映
		for (auto& m : materials)
		{
			auto& p = m.second;
			p->texTilling = texTilling;
			p->shiftUv = shiftUv;
			p->ambient = ambient;
			p->diffuse = diffuse;
			p->specular = specular;
			p->update();
		}
		materialDirty = false;
	}

	// デスクリプタヒープの配列
	if (descHeap)
	{
		ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
		cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}

	// 全メッシュを描画
	for (auto& mesh : meshes)
	{
		mesh->draw(cmdList);
	}
}