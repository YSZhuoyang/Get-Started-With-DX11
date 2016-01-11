#pragma once
#include <fbxsdk.h>

using namespace DirectX;
using namespace std;

namespace Utilities
{
	FbxAMatrix GetTransformMatrix(FbxNode* node);
	void ConvertFbxAMatrixToDXMatrix(XMFLOAT4X4* outMatrix, FbxAMatrix fbxAMatrix);
};
