#pragma once
#include <vector>
#include <string>
#include <fbxsdk.h>

#include "pch.h"
#include "Content\ShaderStructures.h"
#include "Common\DeviceResources.h"
#include "Common\DirectXHelper.h"
#include "Common\StepTimer.h"


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

		pair<unsigned int, float> boneWeight[4];
	};

	// Change name Bone to Joint, linkedNode to bone?
	struct Bone
	{
	public:
		XMFLOAT4X4 GetBoneMatrix(unsigned int frame);

		int boneIndex;
		int parentIndex;
		string name;
		FbxAMatrix globalBindposeInverseMatrix;
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
		void UpdateMeshMatrix(unsigned int frame);

		unsigned int numIndices;
		unsigned int numVertices;

		ComPtr<ID3D11Buffer> vertexBuffer;
		//ComPtr<ID3D11Buffer> indexBuffer;
		ComPtr<ID3D11ShaderResourceView> srv;

		vector<Vertex> vertices;
		vector<unsigned int> indices;

		FbxNode* fbxNode;
		XMFLOAT4X4 globalMeshBaseMatrix;
		XMFLOAT4X4 globalMeshTransform;
	};

	class ModelObj
	{
	public:
		ModelObj();
		void InitMesh(ID3D11Device3* device);
		void InitAnimationData(ID3D11Device3* device);
		/*void ComputeClusterDeformation(
			FbxMesh* pMesh,
			FbxCluster* pCluster,
			FbxAMatrix& pVertexTransformMatrix,
			FbxTime pTime,
			FbxPose* pPose);*/
		void Render(ID3D11DeviceContext3* context, ID3D11SamplerState* sampleState);
		void Update(StepTimer const& timer);
		void Release();

		// Animation data
		FbxTime start;
		FbxTime end;
		unsigned int duration;

		// file info
		string fileName;
		string path;

		unsigned short numMesh;
		XMFLOAT4X4 modelMatrix;
		//FbxPose* fbxPose;
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

		//bool triangulated;
		//unsigned short numNodes;

		//ID3D11Device3* device;
		//ID3D11DeviceContext3* context;
	};
}
