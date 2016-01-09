#pragma once
#include <vector>
#include <string>
#include <fbxsdk.h>

#include "pch.h"
#include "Content\ShaderStructures.h"
#include "Common\DeviceResources.h"
#include "Common\DirectXHelper.h"


using namespace std;
using namespace Microsoft::WRL;
using namespace DX11Sample;
using namespace DirectX;
using namespace DX;

namespace ModelImporter
{
	// Used for reading weights
	struct VertexWeight
	{
	public:
		pair<int, float> boneWeight[4];

		void AddBoneData(unsigned int index, float weight);
		void Normalize();
	};

	// Change name Bone to Joint, linkedNode to bone?
	struct Bone
	{
	public:
		int boneIndex;
		int parentIndex;
		string name;
		//FbxAMatrix globalBindposeInverseMatrix;
		XMFLOAT4X4 globalBoneBaseMatrix;
		FbxNode* fbxNode;

		XMFLOAT4X4 GetBoneMatrix(float frame);
	};

	struct Skeleton
	{
	public:
		vector<Bone> bones;

		Bone* FindBoneByName(string boneName);
	};

	struct MeshEntry
	{
		/*void Init(
		const vector<Vertex>& Vertices,
		const vector<int>& indices,
		double numVertices,
		double numIndices);*/
		void InitResources(ID3D11Device3* device);
		XMFLOAT4X4 GetMeshMatrix(float frame);

		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;
		ComPtr<ID3D11ShaderResourceView> srv;
		//ComPtr<ID3D11InputLayout> m_inputLayout;

		unsigned int numIndices;
		unsigned int numVertices;
		//unsigned int materialIndex;

		vector<Vertex> vertices;
		vector<unsigned int> indices;

		FbxNode* fbxNode;
		XMFLOAT4X4 globalMeshBaseMatrix;

		//Skeleton skeleton;

		//D3D11_INPUT_ELEMENT_DESC vertexDesc[];
	};

	class ModelObj
	{
	public:
		ModelObj();
		void InitMesh(ID3D11Device3* device);
		void Render(ID3D11DeviceContext3* context, ID3D11SamplerState* sampleState);
		void Release();

		Skeleton* skeleton;
		//FbxScene* fbxScene;
		vector<MeshEntry> entries;
		//map<string, int> nodeInfo;

	private:
		void PrintNode(FbxNode* node);
		void PrintNodeAttribute(FbxNodeAttribute* attr);
		void Clear();
		FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);

		// Cached pointer to device resources.
		//std::shared_ptr<DX::DeviceResources> m_deviceResources;

		bool triangulated;
		unsigned short numNodes;

		//ID3D11Device3* device;
		//ID3D11DeviceContext3* context;
	};
}
