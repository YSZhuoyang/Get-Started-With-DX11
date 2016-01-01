#pragma once

#include "MeshImporter.h"

namespace ModelImporter
{
	class FbxLoader
	{
	public:
		FbxLoader();
		~FbxLoader();

		void CreateFbxResources();
		void ReleaseFbxResources();
		void LoadFbxModel(const char* fileName, ModelObj* model, ID3D11Device3* device,
			ID3D11DeviceContext3* context);

	private:
		FbxManager* fbxManager;
		FbxIOSettings* ios;
		FbxScene* scene;
		FbxImporter* importer;

		MeshImporter* meshImporter;

		bool triangulated;
	};
}
