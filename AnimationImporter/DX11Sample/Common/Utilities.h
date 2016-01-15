#pragma once
#include <fbxsdk.h>
#include "DirectXHelper.h"
#include "Common\ModelLoader\ModelObj.h"

using namespace DirectX;
using namespace DX;
using namespace std;
using namespace ModelImporter;


namespace Utilities
{
	FbxAMatrix GetTransformMatrix(FbxNode* node);
	FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
	FbxAMatrix GetGlobalPosition(
		FbxNode* pNode,
		const FbxTime& pTime,
		FbxPose* pPose,
		FbxAMatrix* pParentGlobalPosition = NULL);
	void ConvertFbxAMatrixToDXMatrix(XMFLOAT4X4* outMatrix, FbxAMatrix fbxAMatrix);
	void const LoadTexture(
		const char* path,
		const char* fileName,
		MeshEntry* mesh,
		bool& isTextured,
		ID3D11Device3* device,
		ID3D11DeviceContext3* context);
};
