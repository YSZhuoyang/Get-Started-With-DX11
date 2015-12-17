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
		void LoadMesh(const string& fileName);
		void Render();

	private:
		void InitFromScene(const string& fileName);
		void InitMesh(unsigned int index);
		void InitMaterials(const string& fileName);
		void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

		struct MeshEntry
		{
			bool Init(const vector<Vertex>& Vertices,
				const vector<int>& indices);

			ComPtr<ID3D11Buffer> vertexBuffer;
			ComPtr<ID3D11Buffer> indexBuffer;

			ComPtr<ID3D11InputLayout> m_inputLayout;


			unsigned int numIndices;
			unsigned int materialIndex;
		};

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		ID3D11Device3* device;
		ID3D11DeviceContext3* context;

		vector<MeshEntry> entries;
		//vector<ID3D11Texture2D> textures;
	};
}
