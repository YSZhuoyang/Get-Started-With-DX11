#pragma once
#include <fbxsdk.h>

using namespace DirectX;
using namespace std;

namespace Utilities
{
	FbxAMatrix GetTransformMatrix(FbxNode* node);
	void ConvertFbxAMatrixToDXMatrix(XMFLOAT4X4* outMatrix, FbxAMatrix fbxAMatrix);
	FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
	FbxAMatrix GetGlobalPosition(
		FbxNode* pNode,
		const FbxTime& pTime,
		FbxPose* pPose,
		FbxAMatrix* pParentGlobalPosition = NULL);
};
