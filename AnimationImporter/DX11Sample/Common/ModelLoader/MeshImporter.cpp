#include "pch.h"

#include <ppltasks.h>
#include <string>

#include "MeshImporter.h"

using namespace ModelImporter;
using namespace DX;

MeshImporter::MeshImporter(ModelObj* modelInput)
{
	model = modelInput;
}

MeshImporter::~MeshImporter()
{
	model = nullptr;
}

void MeshImporter::LoadMesh(FbxScene* scene, ID3D11Device3* device,
	ID3D11DeviceContext3* context)
{
	PrintTab("Start load meshes");

	// Obtain root node
	FbxNode* root = scene->GetRootNode();

	if (root)
	{
		// Root node is included
		numNodes = root->GetChildCount(true) + 1;

		model->entries.clear();
		model->entries.reserve(numNodes);

		PrintTab("Number of nodes: " + to_string(numNodes));

		LoadNodeMesh(root, device, context);
	}

	PrintTab("End load meshes");
}

void MeshImporter::LoadNodeMesh(FbxNode* node, ID3D11Device3* device,
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

		// Do not use indexed drawing method
		numVertices = numIndices;

		vector<Vertex> vertices(numVertices);
		vector<unsigned int> indices(numIndices);

		numPolygonVert = 3;
		//assert(numPolygonVert == 3);

		FbxVector4* controlPoints = fbxMesh->GetControlPoints();

		int* indices_array = fbxMesh->GetPolygonVertices();

		// Need to be changed for optimization
		for (unsigned int i = 0; i < numIndices; i++)
		{
			indices[i] = indices_array[i];

			vertices[i].pos.x = (float)fbxMesh->GetControlPointAt(indices[i]).mData[0] / 10000.0f;
			vertices[i].pos.y = (float)fbxMesh->GetControlPointAt(indices[i]).mData[1] / 10000.0f;
			vertices[i].pos.z = (float)fbxMesh->GetControlPointAt(indices[i]).mData[2] / 10000.0f;
		}

		// For indexed drawing
		/*for (unsigned int i = 0; i < numVertices; i++)
		{
		vertices[i].pos.x = (float)controlPoints[i].mData[0];// / 25.0f;
		vertices[i].pos.y = (float)controlPoints[i].mData[1];// / 25.0f;
		vertices[i].pos.z = (float)controlPoints[i].mData[2];// / 25.0f;
		}*/

		LoadUV(fbxMesh, &vertices[0], &indices[0]);

		// Set to be clockwise, must be done after reading uvs and normals
		for (auto it = vertices.begin(); it != vertices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}

		//OutputDebugStringA(("\n number of polygons: " + to_string(numPolygons) + " \n").c_str());
		//OutputDebugStringA(("\n number of indices: " + to_string(numIndices) + " \n").c_str());
		//OutputDebugStringA(("\n number of vertices: " + to_string(vertices.size()) + " \n").c_str());

		ModelObj::MeshEntry mesh;
		mesh.vertices = vertices;
		mesh.indices = indices;
		mesh.numVertices = numVertices;
		mesh.numIndices = numIndices;

		LoadMaterials(node, &mesh, device, context);

		model->entries.push_back(mesh);
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		LoadNodeMesh(node->GetChild(i), device, context);
	}
}

void MeshImporter::LoadUV(FbxMesh* fbxMesh, Vertex* vertices, unsigned int* indices)
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

					// get the index of the current vertex in control points array
					int polyVertIndex = fbxMesh->GetPolygonVertex(polyIndex, vertIndex);

					// the UV index depends on the reference mode
					int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyVertIndex) : polyVertIndex;

					UVValue = UVElement->GetDirectArray().GetAt(UVIndex);

					// Copy texture coordinates
					// For indexed drawing
					//unsigned int vertexIndex = indices[polyVertIndex];

					vertices[polyVertIndex].uv.x = (float)UVValue.mData[0];
					vertices[polyVertIndex].uv.y = 1.0f - (float)UVValue.mData[1];
				}
			}

			PrintTab("UV got eByControlPoint!!");
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

						// the UV index depends on the reference mode
						int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyIndexCounter) : polyIndexCounter;

						UVValue = UVElement->GetDirectArray().GetAt(UVIndex);

						// Copy texture coordinates
						// For indexed drawing
						//unsigned int vertexIndex = indices[polyIndexCounter];

						vertices[polyIndexCounter].uv.x = (float)UVValue.mData[0];
						vertices[polyIndexCounter].uv.y = 1.0f - (float)UVValue.mData[1];

						polyIndexCounter++;
					}
				}
			}

			PrintTab("UV got eByPolygonVertex!!");
		}
	}
}

void MeshImporter::LoadMaterials(FbxNode* node, ModelObj::MeshEntry* mesh, ID3D11Device3* device,
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
						LoadTexture(texture_name, mesh, device, context);

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
					LoadTexture(texture_name, mesh, device, context);

					PrintTab(to_string(texture_count) + " Single texture loaded!");
				}
			}
		}
	}
}

void const MeshImporter::LoadTexture(const char* fileName, ModelObj::MeshEntry* mesh, ID3D11Device3* device,
	ID3D11DeviceContext3* context)
{
	PrintTab("Start load texture file");
	PrintTab(fileName);

	string path = "Assets\\starwars-millennium-falcon\\";
	//string path("Assets\\farm_house\\Textures\\");
	//string path = "Assets\\Wooden_House\\";

	//fileName = "Farmhouse Texture.jpg";
	string fileNameStr(fileName);

	// For testing
	/*if (fileNameStr.find(".png") == -1)
	{
	fileNameStr += ".png";
	}*/

	HRESULT hr = CreateWICTextureFromFile(device, context, GetWC((path + fileNameStr).c_str()),
		nullptr, mesh->srv.GetAddressOf());

	if (FAILED(hr))
	{
		// Try both uppercase and lowercase
		HRESULT hr2 = CreateWICTextureFromFile(device, context,
			GetWC((path + GetLower(fileNameStr.c_str())).c_str()),
			nullptr, mesh->srv.GetAddressOf());

		if (FAILED(hr2))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	PrintTab("End load texture file");
}
