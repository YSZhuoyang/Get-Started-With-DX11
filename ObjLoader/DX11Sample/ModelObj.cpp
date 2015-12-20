#include "pch.h"
#include <ppltasks.h>
#include <string>

#include "ModelObj.h"

using namespace Custom;
using namespace DirectX;
using namespace DX;
using namespace std;
using namespace Windows;

ModelObj::ModelObj()
{
	//device = m_deviceResources->GetD3DDevice();
	//context = m_deviceResources->GetD3DDeviceContext();
}

void ModelObj::LoadModel(const char* fileName)
{
	PrintTab("Start load file");

	//Clear();

	// create a SdkManager
	FbxManager* fbxManager = FbxManager::Create();
	// create an IOSettings object
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
	// create an empty scene
	FbxScene* scene = FbxScene::Create(fbxManager, "myScene");
	// create an importer.
	FbxImporter* importer = FbxImporter::Create(fbxManager, "");

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

	importer->Destroy();

	// Triangulate all geometries
	FbxGeometryConverter lGeomConverter(fbxManager);
	
	if (lGeomConverter.Triangulate(scene, /*replace*/true))
	{
		PrintTab("Triangulated");
	}

	LoadMesh(scene);

	fbxManager->Destroy();

	PrintTab("End load file");
}

void ModelObj::LoadMesh(FbxScene* scene)
{
	PrintTab("Start load meshes");

	// Obtain root node
	FbxNode* root = scene->GetRootNode();

	if (root)
	{
		numMeshes = root->GetChildCount(true);

		entries.clear();
		entries.reserve(numMeshes + 1);

		PrintTab("Number of meshes: " + to_string(numMeshes));

		LoadNodeMesh(root);

		/*unsigned int numChild = root->GetChildCount();

		for (unsigned int i = 0; i < numChild; i++)
		{
			LoadNodeMesh(root->GetChild(i));
		}*/
	}

	PrintTab("End load meshes");
}

void ModelObj::LoadNodeMesh(FbxNode* node)
{
	unsigned int numPolygons = 0;
	unsigned int numVertices = 0;
	unsigned int numIndices = 0;
	unsigned int numPolygonVert = 0;

	if (node->GetNodeAttribute() != NULL &&
		node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		//PrintNode(node);
		
		// Create meshes
		FbxMesh* fbxMesh = node->GetMesh();
		numPolygons = fbxMesh->GetPolygonCount();
		numIndices = fbxMesh->GetPolygonVertexCount();
		numVertices = fbxMesh->GetControlPointsCount();

		/*if (numVertices == 0)
		{
			PrintTab("Empty mesh");

			continue;
		}

		for (int i = 0; i < numPolygons; i++)
		{
			PrintTab(to_string(fbxMesh->GetPolygonSize(i)));
		}*/

		vector<Vertex> vertices(numVertices);
		vector<int> indices(numIndices);

		numPolygonVert = 3;
		//assert(numPolygonVert == 3);

		FbxVector4* controlPoints = fbxMesh->GetControlPoints();
		int* indices_array = fbxMesh->GetPolygonVertices();

		// Need to be changed for optimization
		for (int i = 0; i < numIndices; i++)
		{
			indices[i] = indices_array[i];
		}

		for (unsigned int i = 0; i < numVertices; i++)
		{
			/*Vertex v(
			(float)fbxMesh->GetControlPointAt(i).mData[0],
			(float)fbxMesh->GetControlPointAt(i).mData[1],
			(float)fbxMesh->GetControlPointAt(i).mData[2]);*/
			//(float)controlPoints[i].mData[0],
			//(float)controlPoints[i].mData[1],
			//(float)controlPoints[i].mData[2]);

			vertices[i].pos.x = (float)controlPoints[i].mData[0];// / 10000.0f;
			vertices[i].pos.y = (float)controlPoints[i].mData[1];// / 10000.0f;
			vertices[i].pos.z = (float)controlPoints[i].mData[2];// / 10000.0f;
		}

		//OutputDebugStringA(("\n number of polygons: " + to_string(numPolygons) + " \n").c_str());
		//OutputDebugStringA(("\n number of indices: " + to_string(numIndices) + " \n").c_str());
		//OutputDebugStringA(("\n number of vertices: " + to_string(vertices.size()) + " \n").c_str());

		/* This method does not use index drawing
		for (unsigned int i = 0; i < numPolygons; i++)
		{
			numPolygonVert = fbxMesh->GetPolygonSize(i);
			assert(numPolygonVert == 3);

			for (unsigned int j = 0; j < numPolygonVert; j++)
			{
				int controlPointIndex = fbxMesh->GetPolygonVertex(i, j);
				Vertex v(
					(float)controlPoints[controlPointIndex].mData[0],
					(float)controlPoints[controlPointIndex].mData[0],
					(float)controlPoints[controlPointIndex].mData[0]);
				vertices.push_back(v);
			}
		}*/

		MeshEntry mesh;
		mesh.vertices = vertices;
		mesh.indices = indices;
		mesh.numVertices = vertices.size();
		mesh.numIndices = indices.size();

		entries.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->GetChildCount(); i++)
	{
		LoadNodeMesh(node->GetChild(i));
	}
}

void ModelObj::MeshEntry::InitResources(ID3D11Device3* device)
{
	PrintTab("Start init resources of a mesh");

	/*D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA}
	};*/

	//device->CreateInputLayout();

	// Create vertex buffer
	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = &vertices[0];
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(numVertices * sizeof(Vertex), D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
			&vertexBufferDesc,
			&vertexBufferData,
			&vertexBuffer
			)
		);

	// Create index buffer
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = &indices[0];
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(numIndices * 4, D3D11_BIND_INDEX_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&indexBuffer
			)
		);

	PrintTab("End init resources of a mesh");
}

void ModelObj::InitMesh(ID3D11Device3* device)
{
	PrintTab("Start init mesh");

	for (unsigned int i = 0; i < entries.size(); i++)
	{
		entries[i].InitResources(device);
	}

	PrintTab("End init mesh");
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

void ModelObj::Render(ID3D11DeviceContext3* context)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//for (unsigned int i = 0; i < entries.size(); i++)
	for (vector<MeshEntry>::iterator mesh = entries.begin(); mesh != entries.end(); ++mesh)
	{
		//MeshEntry mesh = entries[i];

		context->IASetVertexBuffers(
			0,
			1,
			mesh->vertexBuffer.GetAddressOf(),
			&stride,
			&offset
			);

		context->IASetIndexBuffer(
			mesh->indexBuffer.Get(),
			DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
			0
			);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//context->IASetInputLayout(mesh->inputLayout.Get());

		context->DrawIndexed(
			mesh->numIndices,
			0,
			0);
	}
}

void ModelObj::PrintNode(FbxNode* node)
{
	PrintTab("/Node");

	const char* nodeName = node->GetName();
	FbxDouble3 translation = node->LclTranslation.Get();
	FbxDouble3 rotation = node->LclRotation.Get();
	FbxDouble3 scaling = node->LclScaling.Get();
	//node->SetGeometricScaling(FbxNode::eDESTINATION_SET, node->GetGeometricScaling(FbxNode::eSOURCE_SET));

	// Print contents of the node
	OutputDebugStringA(("Position: " +
		to_string(translation[0]) + ", " +
		to_string(translation[1]) + ", " +
		to_string(translation[2]) + ";\n").c_str());
	OutputDebugStringA(("Rotation: " +
		to_string(rotation[0]) + ", " +
		to_string(rotation[1]) + ", " +
		to_string(rotation[2]) + ";\n").c_str());
	OutputDebugStringA(("Scaling: " +
		to_string(scaling[0]) + ", " +
		to_string(scaling[1]) + ", " +
		to_string(scaling[2]) + ";\n").c_str());

	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		PrintNodeAttribute(node->GetNodeAttributeByIndex(i));
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		PrintNode(node->GetChild(i));
	}

	PrintTab("/Node");
}

void ModelObj::PrintNodeAttribute(FbxNodeAttribute* attr)
{
	if (!attr)
	{
		return;
	}
	
	//Doesn't support UWP API
	//string typeName = GetAttributeTypeName(attr->GetAttributeType());
	string attrName = attr->GetName();

	OutputDebugStringA(("\nAttr type: " + to_string(attr->GetAttributeType()) + "\n").c_str());
	OutputDebugStringA(("Attr name: " + attrName + "\n").c_str());
}

FbxString GetAttributeTypeName(FbxNodeAttribute::EType type)
{
	switch (type)
	{
		case FbxNodeAttribute::eUnknown: return "unidentified";
		case FbxNodeAttribute::eNull: return "null";
		case FbxNodeAttribute::eMarker: return "marker";
		case FbxNodeAttribute::eSkeleton: return "skeleton";
		case FbxNodeAttribute::eMesh: return "mesh";
		case FbxNodeAttribute::eNurbs: return "nurbs";
		case FbxNodeAttribute::ePatch: return "patch";
		case FbxNodeAttribute::eCamera: return "camera";
		case FbxNodeAttribute::eCameraStereo: return "stereo";
		case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
		case FbxNodeAttribute::eLight: return "light";
		case FbxNodeAttribute::eOpticalReference: return "optical reference";
		case FbxNodeAttribute::eOpticalMarker: return "marker";
		case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
		case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
		case FbxNodeAttribute::eBoundary: return "boundary";
		case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
		case FbxNodeAttribute::eShape: return "shape";
		case FbxNodeAttribute::eLODGroup: return "lodgroup";
		case FbxNodeAttribute::eSubDiv: return "subdiv";
		default: return "unknown";
	}
}

void ModelObj::Clear()
{

}

/*void ModelObj::MeshEntry::Init(
	const vector<Vertex>& Vertices, 
	const vector<int>& Indices, 
	double NumVertices, 
	double NumIndices)
{
	vertices = Vertices;
	indices = Indices;
	numVertices = NumVertices;
	numIndices = NumIndices;
}*/


