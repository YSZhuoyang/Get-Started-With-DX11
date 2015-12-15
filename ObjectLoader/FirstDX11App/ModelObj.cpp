#include "pch.h"
#include "ModelObj.h"


ModelObj::ModelObj()
{
}


bool ModelObj::LoadMesh( string& fileName)
{
	bool loadedSuccessfully;
	Assimp::Importer importer;

	Clear();

	const aiScene* pScene = importer.ReadFile(fileName.c_str(), 
		aiProcess_Triangulate | 
		aiProcess_GenSmoothNormals | 
		aiProcess_FlipUVs);

	if (pScene)
	{
		loadedSuccessfully = InitFromScene(pScene, fileName);
	}
	else
	{
		printf("Error parsing '%s': '%s'\n", 
			fileName.c_str(), 
			importer.GetErrorString());
	}

	return loadedSuccessfully;
}

bool ModelObj::InitFromScene(const aiScene* pScene, const string& fileName)
{
	entries.resize(pScene->mNumMeshes);
	//textures.resize(pScene->mNumMaterials);

	for (unsigned int i = 0; i < entries.size(); i++)
	{
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh);
	}

	return InitMaterials(pScene, fileName);
}

void ModelObj::InitMesh(unsigned int index, const aiMesh* paiMesh)
{
	entries[index].materialIndex = paiMesh->mMaterialIndex;

	vector<Vertex> vertices;
	vector<int> indices;

	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = paiMesh->HasNormals()?
			&(paiMesh->mNormals[i]) : &Zero3D;
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ?
			&(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Vertex v(pPos->x, pPos->y, pPos->z);
		// TexCoords, Normals ...

		vertices.push_back(v);
	}

	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace& face = paiMesh->mFaces[i];

		assert(face.mNumIndices == 3);

		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	entries[index].Init(vertices, indices);
}

bool ModelObj::InitMaterials(const aiScene* pScene, const string& fileName)
{
	for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		
		/*textures[i] = NULL;
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path,
				NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				string fullPath = Dir + "/" + path.data;
				textures[i] = new ID3D11Texture3D()...

			}
		}*/
	}
}
