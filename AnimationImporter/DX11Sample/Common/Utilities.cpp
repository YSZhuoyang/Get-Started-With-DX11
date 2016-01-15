#include "pch.h"
#include "Utilities.h"
#include "DirectXHelper.h"

using namespace DX;

namespace Utilities
{
	// Get the global position of the node for the current pose.
	// If the specified node is not part of the pose or no pose is specified, get its
	// global position at the current time.
	FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition)
	{
		FbxAMatrix lGlobalPosition;
		bool lPositionFound = false;

		if (pPose)
		{
			int lNodeIndex = pPose->Find(pNode);

			if (lNodeIndex > -1)
			{
				// The bind pose is always a global matrix.
				// If we have a rest pose, we need to check if it is
				// stored in global or local space.
				if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
				{
					lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
				}
				else
				{
					// We have a local matrix, we need to convert it to
					// a global space matrix.
					FbxAMatrix lParentGlobalPosition;

					if (pParentGlobalPosition)
					{
						lParentGlobalPosition = *pParentGlobalPosition;
					}
					else
					{
						if (pNode->GetParent())
						{
							lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
						}
					}
					FbxAMatrix lLocalPosition = pNode->GetScene()->GetAnimationEvaluator()->GetNodeLocalTransform(pNode, pTime);
					//FbxAnimEvaluator::GetNodeLocalTransform(pNode, pTime);
					//FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
					lGlobalPosition = lParentGlobalPosition * lLocalPosition;
				}

				PrintTab("Global bone mat2: ");
				lPositionFound = true;
			}
		}

		if (!lPositionFound)
		{
			// There is no pose entry for that node, get the current global position instead.
			// Ideally this would use parent global position and local position to compute the global position.
			// Unfortunately the equation 
			//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
			// does not hold when inheritance type is other than "Parent" (RSrs).
			// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
			lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
		}

		return lGlobalPosition;
	}

	// Get the matrix of the given pose
	FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
	{
		FbxAMatrix lPoseMatrix;
		FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);
		memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

		return lPoseMatrix;
	}

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


