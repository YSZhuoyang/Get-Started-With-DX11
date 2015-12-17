#include "pch.h"
#include "ModelObj.h"

using namespace Custom;
using namespace DirectX;
using namespace DX;


ModelObj::ModelObj()
{
	device = m_deviceResources->GetD3DDevice();
	context = m_deviceResources->GetD3DDeviceContext();
}

void ModelObj::LoadMesh(const string& fileName)
{
	//Clear();

	FbxManager* fbxManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);

	// Create importer
	FbxImporter* importer = FbxImporter::Create(fbxManager, "");

	// Use the first argument as the filename for the importer
	if (!importer->Initialize(fileName.c_str, -1, fbxManager->GetIOSettings()))
	{
		string debug_message = "Call to FbxImporter::Initialize() failed.\n";
		string error = importer->GetStatus().GetErrorString();

		OutputDebugString((debug_message + error).c_str);
		exit(-1);
	}

	// Create scene and import content of the file into the scene
	FbxScene* scene = FbxScene::Create(fbxManager, "myScene");
	importer->Import(scene);
	importer->Destroy();

	// Obtain root node
	FbxNode* root = scene->GetRootNode();

	if (root)
	{
		for (int i = 0; i < root->GetChildCount(); i++)
		{
			//PrintNode(root->GetChild(i));

			// Create meshes

		}
	}

	fbxManager->Destroy();

	//size_t nFaces = objLoader->indices.size() / 3;
	//size_t nVerts = objLoader->vertices.size();
}

void ModelObj::InitFromScene(const string& fileName)
{
	//entries.resize(pScene->mNumMeshes);
	//textures.resize(pScene->mNumMaterials);

	/*for (unsigned int i = 0; i < entries.size(); i++)
	{
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh);
	}*/

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA}
	};

	//device->CreateInputLayout()

	//return InitMaterials(pScene, fileName);
}

void ModelObj::InitMesh(unsigned int index)
{
	/*entries[index].materialIndex = paiMesh->mMaterialIndex;

	vector<Vertex> vertices;
	vector<int> indices;

	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = paiMesh->HasNormals() ?
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

	entries[index].Init(vertices, indices);*/
}

void ModelObj::InitMaterials(const string& fileName)
{
	/*bool initSuccessfully = true;

	for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		ComPtr<ID3D11Resource> resource;
		//ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> srv;
		
		HRESULT hr = CreateDDSTextureFromFile(device,
			L"SEAFLOOR.DDS", //fileName.c_str,
			resource.GetAddressOf(), 
			srv.GetAddressOf());

		DX::ThrowIfFailed(hr);

		D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
		resource->GetType(&resType);

		//resource.As(&texture);

		switch (resType)
		{
			case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			{
				ComPtr<ID3D11Texture1D> tex;
				hr = resource.As(&tex);
				DX::ThrowIfFailed(hr);

				D3D11_TEXTURE1D_DESC desc;
				tex->GetDesc(&desc);

				// This is a 1D texture. Check values of desc here
			}
			break;

			case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				ComPtr<ID3D11Texture2D> tex;
				hr = resource.As(&tex);
				DX::ThrowIfFailed(hr);

				D3D11_TEXTURE2D_DESC desc;
				tex->GetDesc(&desc);

				//textures[i] = tex;

				// This is a 2D texture. Check values of desc here
			}
			break;

			case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			{
				ComPtr<ID3D11Texture3D> tex;
				hr = resource.As(&tex);
				DX::ThrowIfFailed(hr);

				D3D11_TEXTURE3D_DESC desc;
				tex->GetDesc(&desc);

				// This is a 3D volume texture. Check values of desc here
			}
			break;

			default:
				// Error!
				break;
		}

		/*textures[i] = NULL;
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
		aiString path;

		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path,
		NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
		string fullPath = Dir + "/" + path.data;
		textures[i] = new ID3D11Texture3D()...

		if (!textures[i]->Load())
		{
		delete textures[i];
		textures[i] = NULL;
		initSuccessfully = false;
		}
		}
		}*/
	/*}

	return initSuccessfully;*/
}

void ModelObj::Render()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//for (unsigned int i = 0; i < entries.size; i++)
	for (vector<MeshEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
	{
		//MeshEntry mesh = entries[i];

		/*context->IASetVertexBuffers(
			0,
			1,
			vertexBuffer.GetAddressOf(),
			&stride,
			&offset
			);

		context->IASetIndexBuffer(
			indexBuffer.Get(),
			DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
			0
			);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->IASetInputLayout(inputLayout.Get());*/
	}
	
	
}

void ModelObj::Clear()
{

}

bool ModelObj::MeshEntry::Init(const vector<Vertex>& Vertices, const vector<int>& indices)
{
	return true;
}


