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

void FbxLoader::LoadFbxModel(const char* fileName, ModelObj* model, ID3D11Device3* device,
	ID3D11DeviceContext3* context)
{
	PrintTab("Start load file");

	//Clear();

	//model->fbxScene = scene;
	
	// Use the first argument as the filename for the importer
	if (!importer->Initialize(fileName, -1, fbxManager->GetIOSettings()))
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

	if (triangulated)
	{
		// Triangulate all geometries
		FbxGeometryConverter geomConverter(fbxManager);

		if (geomConverter.Triangulate(scene, /*replace*/true))
		{
			PrintTab("Triangulated");
		}
	}

	meshImporter = new MeshImporter(model);
	animImporter = new AnimationImporter(model);

	meshImporter->LoadMesh(scene, device, context);
	//animImporter->DisplayAnimation(scene, importer);
	
	//fbxManager->Destroy();

	PrintTab("End load file");
}

