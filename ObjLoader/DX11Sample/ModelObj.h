#pragma once
#include <vector>
#include <fbxsdk.h>

#include "pch.h"
#include "Content\ShaderStructures.h"
#include "Common\DeviceResources.h"
#include "Common\DirectXHelper.h"


using namespace std;
using namespace Microsoft::WRL;
using namespace DX11Sample;
using namespace DirectX;

namespace Custom
{
	class ModelObj
	{
	public:
		ModelObj();
		void LoadModel(const char* fileName);
		void LoadMesh(FbxScene* scene);
		void LoadNodeMesh(FbxNode* node);
		void InitMesh(ID3D11Device3* device);
		void Render(ID3D11DeviceContext3* context);

	private:
		void InitMaterials(const string& fileName);
		void PrintNode(FbxNode* node);
		void PrintNodeAttribute(FbxNodeAttribute* attr);
		void Clear();
		FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
		
#define INVALID_MATERIAL 0xFFFFFFFF

		struct MeshEntry
		{
			/*void Init(
				const vector<Vertex>& Vertices,
				const vector<int>& indices, 
				double numVertices, 
				double numIndices);*/
			void InitResources(ID3D11Device3* device);

			ComPtr<ID3D11Buffer> vertexBuffer;
			ComPtr<ID3D11Buffer> indexBuffer;
			ComPtr<ID3D11InputLayout> m_inputLayout;

			double numIndices;
			double numVertices;
			//unsigned int materialIndex;

			vector<Vertex> vertices;
			vector<int> indices;

			//D3D11_INPUT_ELEMENT_DESC vertexDesc[];
		};

		// Cached pointer to device resources.
		//std::shared_ptr<DX::DeviceResources> m_deviceResources;

		unsigned int numMeshes;

		//ID3D11Device3* device;
		//ID3D11DeviceContext3* context;

		vector<MeshEntry> entries;
		//vector<ID3D11Texture2D> textures;
	};
}
