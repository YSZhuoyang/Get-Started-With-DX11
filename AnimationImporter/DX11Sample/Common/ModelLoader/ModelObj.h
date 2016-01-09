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
		void AddBoneData(unsigned int index, float weight);
		void Normalize();

		pair<int, float> boneWeight[4];
	};

	// Change name Bone to Joint, linkedNode to bone?
	struct Bone
	{
	public:
		XMFLOAT4X4 GetBoneMatrix(float frame);

		int boneIndex;
		int parentIndex;
		string name;
		//FbxAMatrix globalBindposeInverseMatrix;
		XMFLOAT4X4 globalBoneBaseMatrix;
		FbxNode* fbxNode;
	};

	struct Skeleton
	{
	public:
		Bone* FindBoneByName(string boneName);

		vector<Bone> bones;
	};

	struct MeshEntry
	{
	public:
		void InitResources(ID3D11Device3* device);
		XMFLOAT4X4 GetMeshMatrix(float frame);

		unsigned int numIndices;
		unsigned int numVertices;

		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;
		ComPtr<ID3D11ShaderResourceView> srv;

		vector<Vertex> vertices;
		vector<unsigned int> indices;

		FbxNode* fbxNode;
		XMFLOAT4X4 globalMeshBaseMatrix;
	};

	class ModelObj
	{
	public:
		ModelObj();
		void InitMesh(ID3D11Device3* device);
		void InitAnimationData();
		void Render(ID3D11DeviceContext3* context, ID3D11SamplerState* sampleState);
		void Release();

		XMFLOAT4X4 globalRootTransform;

		Skeleton* skeleton;
		vector<MeshEntry> entries;
		AnimationConstantBuffer animMatrixBufferData;
		ComPtr<ID3D11Buffer> animConstantBuffer;

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
