#pragma once
#include <fbxsdk.h>

#include "Common\ModelLoader\ModelObj.h"
#include "Common\DirectXHelper.h"

using namespace DX;

namespace ModelImporter
{
	class AnimationImporter
	{
	public:
		AnimationImporter(ModelObj* modelImported);
		~AnimationImporter();

		void DisplayAnimation(FbxScene* scene, FbxImporter* fbxImporter);
		void DisplayAnimation(FbxAnimStack* animStack, FbxNode* node, bool isSwitcher = false);
		void DisplayAnimation(FbxAnimLayer* animLayer, FbxNode* node, bool isSwitcher = false);
		void DisplayChannels(FbxNode* node, FbxAnimLayer* animLayer, void(*DisplayCurve) (FbxAnimCurve* curve), void(*DisplayListCurve) (FbxAnimCurve* curve, FbxProperty* property), bool isSwitcher);
		void LoadSkeleton(FbxNode* root);
		void LoadSkeletonHierarchyRecursively(FbxNode* child, int depth, int index, int parentIndex);
		void ReadAnimCurves(FbxNode* node);
		void AddBoneData(int vertIndex, int boneIndex, float weight);
		/*void DisplayCurveKeys(FbxAnimCurve* curve);
		void DisplayListCurveKeys(FbxAnimCurve* curve, FbxProperty* property);*/
		

	private:
		ModelObj* model;
		Skeleton* skeleton;
	};
}
