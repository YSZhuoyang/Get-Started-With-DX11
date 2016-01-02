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

		void DisplayAnimation(FbxScene* scene);
		void DisplayAnimation(FbxAnimStack* animStack, FbxNode* node, bool isSwitcher = false);
		void DisplayAnimation(FbxAnimLayer* animLayer, FbxNode* node, bool isSwitcher = false);
		void DisplayChannels(FbxNode* node, FbxAnimLayer* animLayer, void(*DisplayCurve) (FbxAnimCurve* curve), void(*DisplayListCurve) (FbxAnimCurve* curve, FbxProperty* property), bool isSwitcher);
		/*void DisplayCurveKeys(FbxAnimCurve* curve);
		void DisplayListCurveKeys(FbxAnimCurve* curve, FbxProperty* property);*/
		

	private:
		ModelObj* model;
	};
}
