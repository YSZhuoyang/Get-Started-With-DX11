#include "pch.h"
#include <ppltasks.h>
#include <string>

#include "ModelObj.h"

using namespace Custom;
using namespace DirectX;
using namespace DX;
using namespace std;
using namespace Windows;

ModelObj::ModelObj() :
	triangulated(true)
{
	//device = m_deviceResources->GetD3DDevice();
	//context = m_deviceResources->GetD3DDeviceContext();
}

void ModelObj::LoadModel(const char* fileName, ID3D11Device3* device,
	ID3D11DeviceContext3* context)
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

	if (triangulated)
	{
		// Triangulate all geometries
		FbxGeometryConverter lGeomConverter(fbxManager);

		if (lGeomConverter.Triangulate(scene, /*replace*/true))
		{
			PrintTab("Triangulated");
		}
	}

	LoadMesh(scene, device, context);

	fbxManager->Destroy();

	PrintTab("End load file");
}

void ModelObj::LoadMesh(FbxScene* scene, ID3D11Device3* device,
	ID3D11DeviceContext3* context)
{
	PrintTab("Start load meshes");

	// Obtain root node
	FbxNode* root = scene->GetRootNode();

	if (root)
	{
		// Root node is included
		numNodes = root->GetChildCount(true) + 1;

		entries.clear();
		entries.reserve(numNodes);

		PrintTab("Number of nodes: " + to_string(numNodes));

		LoadNodeMesh(root, device, context);
	}

	PrintTab("End load meshes");
}

void ModelObj::LoadNodeMesh(FbxNode* node, ID3D11Device3* device,
	ID3D11DeviceContext3* context)
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

		vector<Vertex> vertices(numVertices);
		vector<unsigned int> indices(numIndices);
		//vector<unsigned int> indices(20000);

		numPolygonVert = 3;
		//assert(numPolygonVert == 3);

		FbxVector4* controlPoints = fbxMesh->GetControlPoints();
		//fbxMesh->GetElementUV();
		int* indices_array = fbxMesh->GetPolygonVertices();

		// Need to be changed for optimization
		for (unsigned int i = 0; i < numIndices; i++)
		{
			indices[i] = indices_array[i];
		}

		/*unsigned int indexOfIndices = 0;

		for (unsigned int i = 0; i < numPolygons; i++)
		{
			numPolygonVert = fbxMesh->GetPolygonSize(i);
			
			for (unsigned int j = 0; j < numPolygonVert; j++)
			{
				PrintTab("index: " + to_string(indexOfIndices));
				indices[indexOfIndices++] = fbxMesh->GetPolygonVertex(i, j);
			}
		}

		numIndices = indexOfIndices;
		*/

		// Obtain texture coordinates (wrong! to be modified)
		//FbxLayerElementArrayTemplate<FbxVector2>* texCoords = 0;
		//fbxMesh->GetTextureUV(&texCoords, FbxLayerElement::eTextureDiffuse);

		for (unsigned int i = 0; i < numVertices; i++)
		{
			vertices[i].pos.x = (float)controlPoints[i].mData[0] / 10000.0f;
			vertices[i].pos.y = (float)controlPoints[i].mData[1] / 10000.0f;
			vertices[i].pos.z = (float)controlPoints[i].mData[2] / 10000.0f;

			//vertices[i].uv.x = (float)texCoords->GetAt(i).mData[0];
			//vertices[i].uv.y = 1.0f - (float)texCoords->GetAt(i).mData[1];
		}

		LoadUV(fbxMesh, &vertices[0], &indices[0]);

		//OutputDebugStringA(("\n number of polygons: " + to_string(numPolygons) + " \n").c_str());
		//OutputDebugStringA(("\n number of indices: " + to_string(numIndices) + " \n").c_str());
		//OutputDebugStringA(("\n number of vertices: " + to_string(vertices.size()) + " \n").c_str());

		/* This approach does not use index drawing
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
		mesh.numVertices = numVertices;
		mesh.numIndices = numIndices;

		InitMaterials(node, &mesh, device, context);

		entries.push_back(mesh);
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		LoadNodeMesh(node->GetChild(i), device, context);
	}
}

void ModelObj::LoadUV(FbxMesh* fbxMesh, Vertex* vertices, unsigned int* indices)
{
	//get all UV set names
	FbxStringList UVSetNameList;
	fbxMesh->GetUVSetNames(UVSetNameList);

	//iterating over all uv sets
	for (int lUVSetIndex = 0; lUVSetIndex < UVSetNameList.GetCount(); lUVSetIndex++)
	{
		//get lUVSetIndex-th uv set
		const char* UVSetName = UVSetNameList.GetStringAt(lUVSetIndex);
		const FbxGeometryElementUV* UVElement = fbxMesh->GetElementUV(UVSetName);

		if (!UVElement)
		{
			continue;
		}
		
		// only support mapping mode eByPolygonVertex and eByControlPoint
		if (UVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			UVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
		{
			return;
		}

		//index array, where holds the index referenced to the uv data
		const bool useIndex = UVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int indexCount = (useIndex) ? UVElement->GetIndexArray().GetCount() : 0;

		//iterating through the data by polygon
		const int polyCount = fbxMesh->GetPolygonCount();

		if (UVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int polyIndex = 0; polyIndex < polyCount; ++polyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const unsigned int polySize = fbxMesh->GetPolygonSize(polyIndex);

				for (unsigned int vertIndex = 0; vertIndex < polySize; ++vertIndex)
				{
					FbxVector2 UVValue;

					//get the index of the current vertex in control points array
					int polyVertIndex = fbxMesh->GetPolygonVertex(polyIndex, vertIndex);

					//the UV index depends on the reference mode
					int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyVertIndex) : polyVertIndex;

					UVValue = UVElement->GetDirectArray().GetAt(UVIndex);

					//Read texture coordinates
					unsigned int vertexIndex = indices[polyVertIndex];

					vertices[vertexIndex].uv.x = (float)UVValue.mData[0];
					vertices[vertexIndex].uv.y = 1.0f - (float)UVValue.mData[1];

					PrintTab("UV got eByControlPoint!!");
				}
			}
		}
		else if (UVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int polyIndexCounter = 0;

			for (int polyIndex = 0; polyIndex < polyCount; ++polyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int polySize = fbxMesh->GetPolygonSize(polyIndex);

				for (int vertIndex = 0; vertIndex < polySize; ++vertIndex)
				{
					if (polyIndexCounter < indexCount)
					{
						FbxVector2 UVValue;

						//the UV index depends on the reference mode
						int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyIndexCounter) : polyIndexCounter;

						UVValue = UVElement->GetDirectArray().GetAt(UVIndex);

						//Read texture coordinates
						unsigned int vertexIndex = indices[polyIndexCounter];

						vertices[vertexIndex].uv.x = (float)UVValue.mData[0];
						vertices[vertexIndex].uv.y = 1.0f - (float)UVValue.mData[1];

						PrintTab("x: " + to_string(vertices[vertexIndex].uv.x));
						PrintTab("y: " + to_string(vertices[vertexIndex].uv.y));

						PrintTab("UV got eByPolygonVertex!!");

						polyIndexCounter++;
					}
				}
			}
		}
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
	CD3D11_BUFFER_DESC indexBufferDesc(numIndices * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);
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

void ModelObj::InitMaterials(FbxNode* node, MeshEntry* mesh, ID3D11Device3* device,
	ID3D11DeviceContext3* context)
{
	int mcount = node->GetSrcObjectCount<FbxSurfaceMaterial>();

	for (int index = 0; index < mcount; index++)
	{
		FbxSurfaceMaterial *material = 
			(FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(index);

		if (material)
		{
			// This only gets the material of type sDiffuse, you 
			// probably need to traverse all Standard Material Property 
			// by its name to get all possible textures.
			FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

			// Check if it's layeredtextures
			int layered_texture_count = prop.GetSrcObjectCount<FbxLayeredTexture>();

			if (layered_texture_count > 0)
			{
				for (int j = 0; j < layered_texture_count; j++)
				{
					FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(j));
					int lcount = layered_texture->GetSrcObjectCount<FbxTexture>();

					for (int k = 0; k < lcount; k++)
					{
						FbxTexture* texture = 
							FbxCast<FbxTexture>(layered_texture->GetSrcObject<FbxTexture>(k));
						// Then, you can get all the properties of the texture, include its name
						const char* texture_name = texture->GetName();

						// Load files
						mesh->LoadTexture(texture_name, device, context);

						PrintTab(to_string(layered_texture_count) + " Layered textures loaded!" + 
							"Number of layers: " + to_string(lcount));
					}
				}
			}
			else
			{
				// Directly get textures
				int texture_count = prop.GetSrcObjectCount<FbxTexture>();

				for (int j = 0; j < texture_count; j++)
				{
					const FbxTexture* texture = 
						FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(j));
					// Then, you can get all the properties of the texture, include its name
					const char* texture_name = texture->GetName();

					// Load file
					mesh->LoadTexture(texture_name, device, context);

					PrintTab(to_string(texture_count) + " Single texture loaded!");
				}
			}
		}
	}
}

void const ModelObj::MeshEntry::LoadTexture(const char* fileName, ID3D11Device3* device, 
	ID3D11DeviceContext3* context)
{
	PrintTab("Start load texture file");
	PrintTab(fileName);

	string path = "Assets\\starwars-millennium-falcon\\";
	//string path("Assets\\Wooden_House\\");
	string fileNameStr(fileName);

	// For testing
	/*if (fileNameStr.find(".png") == -1 && fileNameStr.find(".png") == -1)
	{
		fileNameStr += ".png";
	}*/

	HRESULT hr = CreateWICTextureFromFile(device, context, GetWC((path + fileNameStr).c_str()),
		nullptr, srv.GetAddressOf());

	if (FAILED(hr))
	{
		// Try both uppercase and lowercase
		HRESULT hr2 = CreateWICTextureFromFile(device, context,
			GetWC((path + GetLower(fileNameStr.c_str())).c_str()),
			nullptr, srv.GetAddressOf());

		if (FAILED(hr2))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	PrintTab("End load texture file");
}

void ModelObj::Render(ID3D11DeviceContext3* context, ID3D11SamplerState* sampleState)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (vector<MeshEntry>::iterator mesh = entries.begin(); mesh != entries.end(); ++mesh)
	{
		context->IASetVertexBuffers(
			0,
			1,
			mesh->vertexBuffer.GetAddressOf(),
			&stride,
			&offset
			);

		context->IASetIndexBuffer(
			mesh->indexBuffer.Get(),
			DXGI_FORMAT_R32_UINT, // Each index is one 32-bit unsigned integer (short).
			0
			);

		// Set the sampler state in the pixel shader.
		context->PSSetSamplers(0, 1, &sampleState);
		context->PSSetShaderResources(0, 1, mesh->srv.GetAddressOf());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
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

void ModelObj::Release()
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


