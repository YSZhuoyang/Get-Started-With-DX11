#pragma once

#include <vector>
#include <fbxsdk.h>

#include "pch.h"
#include "Content\ShaderStructures.h"
#include "Common\DeviceResources.h"
#include "Common\DirectXHelper.h"
#include "Common\ModelLoader\ModelObj.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace DX11Sample;
using namespace DirectX;

namespace ModelImporter
{
	class MeshImporter
	{
	public:
		MeshImporter(ModelObj* modelInput);
		~MeshImporter();

		void LoadMesh(FbxScene* scene, ID3D11Device3* device,
			ID3D11DeviceContext3* context);
		void LoadNodeMesh(FbxNode* node, ID3D11Device3* device,
			ID3D11DeviceContext3* context);
		void LoadUV(FbxMesh* fbxMesh, Vertex* vertices, unsigned int* indices);
		void LoadMaterials(FbxNode* node, ModelObj::MeshEntry* mesh, ID3D11Device3* device,
			ID3D11DeviceContext3* context);
		void const LoadTexture(const char* fileName, ModelObj::MeshEntry* mesh, ID3D11Device3* device,
			ID3D11DeviceContext3* context);

	private:
		unsigned short numNodes;

		ModelObj* model;

		//ID3D11Device3* device;
		//ID3D11DeviceContext3* context;
	};
}
