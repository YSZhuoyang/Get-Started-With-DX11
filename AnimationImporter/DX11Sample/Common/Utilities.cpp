#include "pch.h"
#include "Utilities.h"

using namespace std;

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
}


