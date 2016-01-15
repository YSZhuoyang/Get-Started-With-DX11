#include "pch.h"
#include "FbxLoader.h"

using namespace ModelImporter;
using namespace DX;

FbxLoader::FbxLoader() :
	triangulated(true)
{
	CreateFbxResources();
}


FbxLoader::~FbxLoader()
{
	ReleaseFbxResources();
}

void FbxLoader::CreateFbxResources()
{
	// create a SdkManager
	fbxManager = FbxManager::Create();
	// create an IOSettings object
	ios = FbxIOSettings::Create(fbxManager, IOSROOT);
	// create an empty scene
	scene = FbxScene::Create(fbxManager, "myScene");
	// create an importer.
	importer = FbxImporter::Create(fbxManager, "");
}

void FbxLoader::ReleaseFbxResources()
{
	meshImporter = nullptr;
	animImporter = nullptr;

	importer->Destroy();
	scene->Destroy();
	ios->Destroy();
	fbxManager->Destroy();
}

void FbxLoader::LoadFbxModel(
	string path, 
	string fileName,
	ModelObj* model, 
	ID3D11Device3* device,
	ID3D11DeviceContext3* context)
{
	PrintTab("Start load file");

	//Clear();

	// Use the first argument as the filename for the importer
	if (!importer->Initialize((path + fileName).c_str(), -1, fbxManager->GetIOSettings()))
	{
		string debug_message = "Call to FbxImporter::Initialize() failed.\n";
		string error = importer->GetStatus().GetErrorString();

		OutputDebugStringA((debug_message + error).c_str());
	}

	// import the scene.
	if (!importer->Import(scene))
	{
		OutputDebugStringA("import failed");
	}

	//importer->Destroy();

	FbxGeometryConverter geomConverter(fbxManager);

	if (triangulated)
	{
		// Triangulate all geometries
		if (geomConverter.Triangulate(scene, /*replace*/true))
		{
			PrintTab("Triangulated");
		}
	}

	// Split meshes per material, so that we only have one material per mesh (for VBO support)
	//geomConverter.SplitMeshesPerMaterial(scene, /*replace*/true);

	// Convert axis system to DirectX
	// Does not convert right hand sys to left hand sys
	/*FbxAxisSystem sceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem myAxisSystem(FbxAxisSystem::DirectX);

	if (sceneAxisSystem != myAxisSystem)
	{
		PrintTab("Converted to DirectX axis system");

		myAxisSystem.ConvertScene(scene);
	}*/

	// Get the current scene units (incoming from FBX file)
	FbxSystemUnit SceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();

	// If the incoming FBX file is defined in centimeters, we have nothing to do
	// so we can skip the conversion...
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		// Force the conversion to centimeters so we make sure
		// that the scale compensation is correctly adjusted
		FbxSystemUnit::cm.ConvertScene(scene);
	}
	
	// Do not take scaling into consideration when child inherits parents transformation
	FbxSystemUnit::ConversionOptions myOptions;
	myOptions.mConvertRrsNodes = false;
	FbxSystemUnit::m.ConvertScene(scene, myOptions);

	model->fileName = fileName;
	model->path = path;

	meshImporter = new MeshImporter(model);
	//animImporter = new AnimationImporter(model);

	//animImporter->DisplayAnimation(scene, importer);
	meshImporter->LoadMesh(scene, device, context);
	
	PrintTab("End load file");
}

