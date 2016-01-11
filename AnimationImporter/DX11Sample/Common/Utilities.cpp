#include "pch.h"
#include "Utilities.h"


namespace Utilities
{
	FbxAMatrix GetTransformMatrix(FbxNode* node)
	{
		if (!node)
		{
			throw exception("Null for mesh geometry!!");
		}

		const FbxVector4 t = node->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 r = node->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 s = node->GetGeometricScaling(FbxNode::eSourcePivot);

		return FbxAMatrix(t, r, s);
	}

	void ConvertFbxAMatrixToDXMatrix(XMFLOAT4X4* outMatrix, FbxAMatrix fbxAMatrix)
	{
		for (int r = 0; r < 4; r++)
		{
			for (int c = 0; c < 4; c++)
			{
				outMatrix->m[r][c] = (float)fbxAMatrix.Get(r, c);
			}
		}
	}
}


