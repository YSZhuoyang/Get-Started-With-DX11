#include "pch.h"
#include "AnimationImporter.h"
#include "Common\Utilities.h"

using namespace ModelImporter;
using namespace Utilities;

AnimationImporter::AnimationImporter(ModelObj* modelImported)
{
	model = modelImported;
	skeleton = new Skeleton();
}

AnimationImporter::~AnimationImporter()
{
	model = nullptr;
}

// Create skeleton objects within a mesh
void AnimationImporter::LoadSkeleton(FbxNode* root)
{
	for (int i = 0; i < root->GetChildCount(); i++)
	{
		FbxNode* child = root->GetChild(i);
		LoadSkeletonHierarchyRecursively(child, 0, 0, -1);
	}

	model->skeleton = skeleton;
}

// Depth value represents the vertical level of the tree, for debugging.
void AnimationImporter::LoadSkeletonHierarchyRecursively(FbxNode* child, int depth, int index, int parentIndex)
{
	if (child->GetNodeAttribute() &&
		child->GetNodeAttribute()->GetAttributeType() &&
		child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Bone bone;
		bone.parentIndex = parentIndex;
		bone.name = child->GetName();

		skeleton->bones.push_back(bone);
	}

	int childCount = child->GetChildCount();

	for (int i = 0; i < childCount; i++)
	{
		LoadSkeletonHierarchyRecursively(child->GetChild(i), depth + 1, skeleton->bones.size(), index);
	}
}

void AnimationImporter::ReadAnimCurves(FbxNode* node)
{
	FbxMesh* fbxMesh = node->GetMesh();
	unsigned int numDeformers = fbxMesh->GetDeformerCount();

	// !!Not sure what is the usage of a geometry transform matrix...
	FbxAMatrix geometryTransform = GetTransformMatrix(node);

	for (unsigned int deformerIndex = 0; deformerIndex < numDeformers; deformerIndex++)
	{
		FbxSkin* fbxSkin = reinterpret_cast<FbxSkin*>(fbxMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

		if (!fbxSkin)
		{
			continue;
		}

		unsigned int numBones = fbxSkin->GetClusterCount();

		for (unsigned int boneIndex = 0; boneIndex < numBones; boneIndex++)
		{
			FbxCluster* fbxCluster = fbxSkin->GetCluster(boneIndex);
			// A line is a joint
			string boneName = fbxCluster->GetLink()->GetName();
			Bone* bone = skeleton->FindBoneByName(boneName);

			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindposeInverseMatrix;

			// Transform of the mesh at binding time
			fbxCluster->GetTransformMatrix(transformMatrix);
			// Transform of the bone from joint space to world space
			fbxCluster->GetTransformLinkMatrix(transformLinkMatrix);
			// !!Not sure what is the usage of a geometry transform matrix...
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

			// Update information in skeleton
			//skeleton->bones[jointIndex].globalBindposeInverseMatrix =
			//	globalBindposeInverseMatrix;
			// The link node is the bone?
			bone->fbxNode = fbxCluster->GetLink();
			
			//PrintTab("Bone linked node: ");
			//PrintTab(fbxCluster->GetLink()->GetName());

			// Associate each joint with control points it affects
			unsigned int numIndices = fbxCluster->GetControlPointIndicesCount();

			for (unsigned int i = 0; i < numIndices; i++)
			{

			}
		}
	}
}

void AnimationImporter::AddBoneData(int vertIndex, int boneIndex, float weight)
{
	
}

void DisplayCurveKeys(FbxAnimCurve* curve);
void DisplayListCurveKeys(FbxAnimCurve* curve, FbxProperty* property);

void AnimationImporter::DisplayAnimation(FbxScene* scene, FbxImporter* fbxImporter)
{
	PrintTab("Start loading animation data");

	// The number of animation frames get 
	auto AnimStackCount = fbxImporter->GetAnimStackCount();
	
	assert(AnimStackCount == 1);
	
	auto takeInfo = fbxImporter->GetTakeInfo(0);

	auto importOffset = takeInfo->mImportOffset;
	auto startTime = takeInfo->mLocalTimeSpan.GetStart();
	auto stopTime = takeInfo->mLocalTimeSpan.GetStop();

	int animationStartFrame = (importOffset.Get() + startTime.Get()) / FbxTime::GetOneFrameValue(FbxTime::eFrames60);
	int animationEndFrame = (importOffset.Get() + stopTime.Get()) / FbxTime::GetOneFrameValue(FbxTime::eFrames60);

	PrintTab("start: " + to_string(animationStartFrame));
	PrintTab("end :" + to_string(animationEndFrame));

	/*for (int i = 0; i < scene->GetSrcObjectCount<FbxAnimStack>(); i++)
	{
		FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);

		FbxString outputString = "Animation Stack Name: ";
		outputString += animStack->GetName();
		outputString += "\n";

		PrintTab(outputString.Buffer());

		DisplayAnimation(animStack, scene->GetRootNode(), true);
		DisplayAnimation(animStack, scene->GetRootNode());

		FbxTakeInfo* animInfo = fbxImporter->GetTakeInfo(i);
		FbxTime startTime = animInfo->mLocalTimeSpan.GetStart();
		FbxTime endTime = animInfo->mLocalTimeSpan.GetStop();
		FbxTime duration = animInfo->mLocalTimeSpan.GetDuration();
	}*/

	PrintTab("End loading animation data");
}


void AnimationImporter::DisplayAnimation(FbxAnimStack* animStack, FbxNode* node, bool isSwitcher)
{
	int numAnimLayers = animStack->GetMemberCount<FbxAnimLayer>();

	FbxString outputString = "Animation stack contains ";
	outputString += numAnimLayers;
	outputString += " Animation Layer(s)";
	
	PrintTab(outputString.Buffer());

	for (int i = 0; i < numAnimLayers; i++)
	{
		FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(i);

		outputString = "AnimLayer ";
		outputString += i;
		PrintTab(outputString.Buffer());

		DisplayAnimation(animLayer, node, isSwitcher);
	}
}

void AnimationImporter::DisplayAnimation(FbxAnimLayer* animLayer, FbxNode* node, bool isSwitcher)
{
	int numChild = node->GetChildCount();

	FbxString outputString;
	outputString = "     Node Name: ";
	outputString += node->GetName();
	outputString += "\n";

	PrintTab(outputString.Buffer());

	DisplayChannels(node, animLayer, DisplayCurveKeys, DisplayListCurveKeys, isSwitcher);

	PrintTab("\n");

	for (int i = 0; i < numChild; i++)
	{
		DisplayAnimation(animLayer, node->GetChild(i), isSwitcher);
	}
}

void AnimationImporter::DisplayChannels(FbxNode* node, FbxAnimLayer* animLayer, void(*DisplayCurve) (FbxAnimCurve* curve), void(*DisplayListCurve) (FbxAnimCurve* curve, FbxProperty* property), bool isSwitcher)
{
	FbxAnimCurve* animCurve = NULL;

	// Display general curves
	if (!isSwitcher)
	{
		animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);

		if (animCurve)
		{
			PrintTab("    TX");
			DisplayCurve(animCurve);
		}

		animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animCurve)
		{
			PrintTab("        TY");
			DisplayCurve(animCurve);
		}

		animCurve = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animCurve)
		{
			PrintTab("        TZ");
			DisplayCurve(animCurve);
		}

		animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);

		if (animCurve)
		{
			PrintTab("        RX");
			DisplayCurve(animCurve);
		}
		
		animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		
		if (animCurve)
		{
			PrintTab("        RY");
			DisplayCurve(animCurve);
		}
		
		animCurve = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		
		if (animCurve)
		{
			PrintTab("        RZ");
			DisplayCurve(animCurve);
		}
		
		animCurve = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
		
		if (animCurve)
		{
			PrintTab("        SX");
			DisplayCurve(animCurve);
		}
		
		animCurve = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		
		if (animCurve)
		{
			PrintTab("        SY");
			DisplayCurve(animCurve);
		}
		
		animCurve = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		
		if (animCurve)
		{
			PrintTab("        SZ");
			DisplayCurve(animCurve);
		}
	}

	// Display curves specific to a light or maker
	FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();

	if (nodeAttribute)
	{
		animCurve = nodeAttribute->Color.GetCurve(animLayer, FBXSDK_CURVENODE_COLOR_RED);

		if (animCurve)
		{
			PrintTab("    Red");
			DisplayCurve(animCurve);
		}

		animCurve = nodeAttribute->Color.GetCurve(animLayer, FBXSDK_CURVENODE_COLOR_GREEN);
		if (animCurve)
		{
			PrintTab("        Green\n");
			DisplayCurve(animCurve);
		}
		animCurve = nodeAttribute->Color.GetCurve(animLayer, FBXSDK_CURVENODE_COLOR_BLUE);
		if (animCurve)
		{
			PrintTab("        Blue\n");
			DisplayCurve(animCurve);
		}

		// Display curves specific to a light.
		FbxLight* light = node->GetLight();

		if (light)
		{
			animCurve = light->Intensity.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Intensity\n");
				DisplayCurve(animCurve);
			}
			animCurve = light->OuterAngle.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Outer Angle\n");
				DisplayCurve(animCurve);
			}
			animCurve = light->Fog.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Fog\n");
				DisplayCurve(animCurve);
			}
		}

		// Display curves specific to a camera
		FbxCamera* camera = node->GetCamera();

		if (camera)
		{
			animCurve = camera->FieldOfView.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Field of View\n");
				DisplayCurve(animCurve);
			}
			animCurve = camera->FieldOfViewX.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Field of View X\n");
				DisplayCurve(animCurve);
			}
			animCurve = camera->FieldOfViewY.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Field of View Y\n");
				DisplayCurve(animCurve);
			}
			animCurve = camera->OpticalCenterX.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Optical Center X\n");
				DisplayCurve(animCurve);
			}
			animCurve = camera->OpticalCenterY.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Optical Center Y\n");
				DisplayCurve(animCurve);
			}
			animCurve = camera->Roll.GetCurve(animLayer);
			if (animCurve)
			{
				PrintTab("        Roll\n");
				DisplayCurve(animCurve);
			}
		}

		// Display curves specific to a geometry
		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			nodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			nodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
		{
			FbxGeometry* geometry = (FbxGeometry*)nodeAttribute;

			int numBlendShapeDeformer = geometry->GetDeformerCount(FbxDeformer::eBlendShape);

			for (int deformerIndex = 0; deformerIndex < numBlendShapeDeformer; deformerIndex++)
			{
				FbxBlendShape* blendShape = (FbxBlendShape*)geometry->GetDeformer(deformerIndex, FbxDeformer::eBlendShape);

				int numBlendShapeChannel = blendShape->GetBlendShapeChannelCount();

				for (int channelIndex = 0; channelIndex < numBlendShapeChannel; channelIndex++)
				{
					FbxBlendShapeChannel* channel = blendShape->GetBlendShapeChannel(channelIndex);

					const char* channelName = channel->GetName();

					animCurve = geometry->GetShapeChannel(deformerIndex, channelIndex, animLayer, true);

					if (animCurve)
					{
						PrintTab("    shape: " + string(channelName));
						DisplayCurve(animCurve);
					}
				}
			}
		}
	}

	// Display curves specific to properties
	FbxProperty property = node->GetFirstProperty();

	while (property.IsValid())
	{
		if (property.GetFlag(FbxPropertyFlags::eUserDefined))
		{
			FbxString fbxCurveNodeName = property.GetName();
			FbxAnimCurveNode* curveNode = property.GetCurveNode(animLayer);

			if (!curveNode)
			{
				property = node->GetNextProperty(property);
				
				continue;
			}

			FbxDataType dataType = property.GetPropertyDataType();

			if (dataType.GetType() == eFbxBool ||
				dataType.GetType() == eFbxDouble ||
				dataType.GetType() == eFbxFloat ||
				dataType.GetType() == eFbxInt)
			{
				FbxString message;

				message = "        Property ";
				message += property.GetName();
				if (property.GetLabel().GetLen() > 0)
				{
					message += " (Label: ";
					message += property.GetLabel();
					message += ")";
				};

				PrintTab(message.Buffer());

				for (int i = 0; i < curveNode->GetCurveCount(0U); i++)
				{
					animCurve = curveNode->GetCurve(0U, i);

					if (animCurve)
					{
						DisplayCurve(animCurve);
					}
				}
			}
			else if (dataType.GetType() == eFbxDouble3 ||
				dataType.GetType() == eFbxDouble4 ||
				dataType.Is(FbxColor3DT) ||
				dataType.Is(FbxColor4DT))
			{
				char* lComponentName1 = (dataType.Is(FbxColor3DT) || dataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_RED : (char*)"X";
				char* lComponentName2 = (dataType.Is(FbxColor3DT) || dataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_GREEN : (char*)"Y";
				char* lComponentName3 = (dataType.Is(FbxColor3DT) || dataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_BLUE : (char*)"Z";
				FbxString      message;

				message = "        Property ";
				message += property.GetName();
				if (property.GetLabel().GetLen() > 0)
				{
					message += " (Label: ";
					message += property.GetLabel();
					message += ")";
				}
				PrintTab(message.Buffer());
				for (int c = 0; c < curveNode->GetCurveCount(0U); c++)
				{
					animCurve = curveNode->GetCurve(0U, c);
					if (animCurve)
					{
						PrintTab("        Component " + string(lComponentName1));
						DisplayCurve(animCurve);
					}
				}
				for (int c = 0; c < curveNode->GetCurveCount(1U); c++)
				{
					animCurve = curveNode->GetCurve(1U, c);
					if (animCurve)
					{
						PrintTab("        Component " + string(lComponentName2));
						DisplayCurve(animCurve);
					}
				}
				for (int c = 0; c < curveNode->GetCurveCount(2U); c++)
				{
					animCurve = curveNode->GetCurve(2U, c);
					if (animCurve)
					{
						PrintTab("        Component " + string(lComponentName3));
						DisplayCurve(animCurve);
					}
				}
			}
			else if (dataType.GetType() == eFbxEnum)
			{
				FbxString message;
				message = "        Property ";
				message += property.GetName();
				
				if (property.GetLabel().GetLen() > 0)
				{
					message += " (Label: ";
					message += property.GetLabel();
					message += ")";
				};

				PrintTab(message.Buffer());
				
				for (int c = 0; c < curveNode->GetCurveCount(0U); c++)
				{
					animCurve = curveNode->GetCurve(0U, c);
					if (animCurve)
						DisplayListCurve(animCurve, &property);
				}
			}
		}

		property = node->GetNextProperty(property);
	}
}

static int InterpolationFlagToIndex(int flags)
{
	if ((flags & FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant) return 1;
	if ((flags & FbxAnimCurveDef::eInterpolationLinear) == FbxAnimCurveDef::eInterpolationLinear) return 2;
	if ((flags & FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic) return 3;
	return 0;
}
static int ConstantmodeFlagToIndex(int flags)
{
	if ((flags & FbxAnimCurveDef::eConstantStandard) == FbxAnimCurveDef::eConstantStandard) return 1;
	if ((flags & FbxAnimCurveDef::eConstantNext) == FbxAnimCurveDef::eConstantNext) return 2;
	return 0;
}
static int TangentmodeFlagToIndex(int flags)
{
	if ((flags & FbxAnimCurveDef::eTangentAuto) == FbxAnimCurveDef::eTangentAuto) return 1;
	if ((flags & FbxAnimCurveDef::eTangentAutoBreak) == FbxAnimCurveDef::eTangentAutoBreak) return 2;
	if ((flags & FbxAnimCurveDef::eTangentTCB) == FbxAnimCurveDef::eTangentTCB) return 3;
	if ((flags & FbxAnimCurveDef::eTangentUser) == FbxAnimCurveDef::eTangentUser) return 4;
	if ((flags & FbxAnimCurveDef::eTangentGenericBreak) == FbxAnimCurveDef::eTangentGenericBreak) return 5;
	if ((flags & FbxAnimCurveDef::eTangentBreak) == FbxAnimCurveDef::eTangentBreak) return 6;
	return 0;
}

static int TangentweightFlagToIndex(int flags)
{
	if ((flags & FbxAnimCurveDef::eWeightedNone) == FbxAnimCurveDef::eWeightedNone) return 1;
	if ((flags & FbxAnimCurveDef::eWeightedRight) == FbxAnimCurveDef::eWeightedRight) return 2;
	if ((flags & FbxAnimCurveDef::eWeightedNextLeft) == FbxAnimCurveDef::eWeightedNextLeft) return 3;
	return 0;
}

static int TangentVelocityFlagToIndex(int flags)
{
	if ((flags & FbxAnimCurveDef::eVelocityNone) == FbxAnimCurveDef::eVelocityNone) return 1;
	if ((flags & FbxAnimCurveDef::eVelocityRight) == FbxAnimCurveDef::eVelocityRight) return 2;
	if ((flags & FbxAnimCurveDef::eVelocityNextLeft) == FbxAnimCurveDef::eVelocityNextLeft) return 3;
	return 0;
}

void DisplayCurveKeys(FbxAnimCurve* curve)
{
	static const char* interpolation[] = { "?", "constant", "linear", "cubic" };
	static const char* constantMode[] = { "?", "Standard", "Next" };
	static const char* cubicMode[] = { "?", "Auto", "Auto break", "Tcb", "User", "Break", "User break" };
	static const char* tangentWVMode[] = { "?", "None", "Right", "Next left" };

	FbxTime   lKeyTime;
	float   lKeyValue;
	char    lTimeString[256];
	FbxString outputString;

	int lKeyCount = curve->KeyGetCount();

	for (int i = 0; i < lKeyCount; i++)
	{
		lKeyValue = static_cast<float>(curve->KeyGetValue(i));
		lKeyTime = curve->KeyGetTime(i);
		
		outputString = "            Key Time: ";
		outputString += lKeyTime.GetTimeString(lTimeString, FbxUShort(256));
		outputString += ".... Key Value: ";
		outputString += lKeyValue;
		outputString += " [ ";
		outputString += interpolation[InterpolationFlagToIndex(curve->KeyGetInterpolation(i))];

		if ((curve->KeyGetInterpolation(i)&FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant)
		{
			outputString += " | ";
			outputString += constantMode[ConstantmodeFlagToIndex(curve->KeyGetConstantMode(i))];
		}
		else if ((curve->KeyGetInterpolation(i)&FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic)
		{
			outputString += " | ";
			outputString += cubicMode[TangentmodeFlagToIndex(curve->KeyGetTangentMode(i))];
			outputString += " | ";
			outputString += tangentWVMode[TangentweightFlagToIndex(curve->KeyGet(i).GetTangentWeightMode())];
			outputString += " | ";
			outputString += tangentWVMode[TangentVelocityFlagToIndex(curve->KeyGet(i).GetTangentVelocityMode())];
		}

		outputString += " ]";
		outputString += "\n";

		PrintTab(outputString.Buffer());
	}
}

void DisplayListCurveKeys(FbxAnimCurve* curve, FbxProperty* property)
{
	FbxTime keyTime;
	int keyValue;
	char timeString[256];
	FbxString listValue;
	FbxString outputString;

	int numKey = curve->KeyGetCount();

	for (int i = 0; i < numKey; i++)
	{
		keyValue = static_cast<int>(curve->KeyGetValue(i));
		keyTime = curve->KeyGetTime(i);

		outputString = "    Key Time: ";
		outputString += keyTime.GetTimeString(timeString, FbxUShort(256));
		outputString += ".... Key Value: ";
		outputString += keyValue;
		outputString += " (";
		outputString += property->GetEnumValue(keyValue);
		outputString += ")";
		outputString += "\n";
		PrintTab(outputString.Buffer());
	}
}

