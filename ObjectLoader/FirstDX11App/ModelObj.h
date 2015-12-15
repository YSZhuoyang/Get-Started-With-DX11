#pragma once
#pragma comment(lib, "assimp.lib")

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing fla
#include <vector>

#include "Content\utils\ShaderDataStructures.h"

using namespace ShaderDataStructure;
using namespace std;
using namespace Microsoft::WRL;

ref class ModelObj sealed
{
public:
	ModelObj();
	bool LoadMesh(const string& fileName);
	void Render();

private:
	bool InitFromScene(const aiScene* pScene, const string& fileName);
	void InitMesh(unsigned int index, const aiMesh* paiMesh);
	bool InitMaterials(const aiScene* pScene, const string& fileName);
	void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

	struct MeshEntry
	{
		MeshEntry();
		~MeshEntry();

		bool Init(const vector<Vertex>& Vertices, 
			const vector<int>& indices);

		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;

		unsigned int numIndices;
		unsigned int materialIndex;
	};

	vector<MeshEntry> entries;
	//vector<ID3D11Texture2D> textures;
};

