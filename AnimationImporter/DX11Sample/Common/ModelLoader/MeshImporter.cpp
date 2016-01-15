#include "pch.h"

#include <ppltasks.h>
#include <string>

#include "MeshImporter.h"
#include "Common\DeviceResources.h"
#include "Common\Utilities.h"

using namespace ModelImporter;
using namespace DX;
using namespace Utilities;

MeshImporter::MeshImporter(ModelObj* modelInput) :
	isTextured(true)
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

	//assert(scene->GetPoseCount() == 1);
	//model->fbxPose = scene->GetPose(0);

	if (root)
	{
		// Root node is included
		model->numMesh = scene->GetMemberCount<FbxMesh>();
		model->entries.clear();
		model->entries.reserve(model->numMesh);

		PrintTab("Number of nodes: " + to_string(root->GetChildCount(true) + 1));

		// Retrieve model matrix
		/*FbxAMatrix fbxGlobalRootTransform = root->EvaluateGlobalTransform();
		XMFLOAT4X4 globalRootTransform;

		ConvertFbxAMatrixToDXMatrix(&globalRootTransform, fbxGlobalRootTransform);*/

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

			vertices[i].pos.x = (float)fbxMesh->GetControlPointAt(indices[i]).mData[0];// / 1000.0f;
			vertices[i].pos.y = (float)fbxMesh->GetControlPointAt(indices[i]).mData[1];// / 1000.0f;
			vertices[i].pos.z = (float)fbxMesh->GetControlPointAt(indices[i]).mData[2];// / 1000.0f;
		}

		// For indexed drawing
		/*for (unsigned int i = 0; i < numVertices; i++)
		{
			vertices[i].pos.x = (float)controlPoints[i].mData[0];
			vertices[i].pos.y = (float)controlPoints[i].mData[1];
			vertices[i].pos.z = (float)controlPoints[i].mData[2];
		}*/

		LoadUV(fbxMesh, &vertices[0], &indices[0]);

		//OutputDebugStringA(("\n number of polygons: " + to_string(numPolygons) + " \n").c_str());
		//OutputDebugStringA(("\n number of indices: " + to_string(numIndices) + " \n").c_str());
		//OutputDebugStringA(("\n number of vertices: " + to_string(vertices.size()) + " \n").c_str());

		// Read mesh base transform matrix
		FbxAMatrix fbxGlobalMeshBaseMatrix = node->EvaluateGlobalTransform().Inverse().Transpose();
		XMFLOAT4X4 globalMeshBaseMatrix;

		// To be considered when importing Maya fbx model
		FbxAMatrix geometricTransform = GetTransformMatrix(node);
		fbxGlobalMeshBaseMatrix *= geometricTransform;

		ConvertFbxAMatrixToDXMatrix(&globalMeshBaseMatrix, fbxGlobalMeshBaseMatrix);

		MeshEntry mesh;
		mesh.vertices = vertices;
		mesh.indices = indices;
		mesh.numVertices = numVertices;
		mesh.numIndices = numIndices;
		mesh.fbxNode = node;
		mesh.globalMeshBaseMatrix = globalMeshBaseMatrix;

		// Load materials and textures
		LoadMaterials(node, &mesh, device, context);

		// Load weights
		LoadWeight(fbxMesh, &mesh);

		// Set to be clockwise, must be done after reading uvs, normals, weights and etc
		ReverseWindingOrder(mesh.vertices);
		
		model->entries.push_back(mesh);
	}

	int numChild = node->GetChildCount();

	for (int i = 0; i < numChild; i++)
	{
		LoadNodeMesh(node->GetChild(i), device, context);
	}
}

void MeshImporter::ReverseWindingOrder(vector<Vertex>& vertices)
{
	for (auto it = vertices.begin(); it != vertices.end(); it += 3)
	{
		swap(*it, *(it + 2));
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

void MeshImporter::LoadWeight(FbxMesh* fbxMesh, MeshEntry* mesh)
{
	int numSkin = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	
	if (numSkin == 0)
	{
		for (auto& vertex : mesh->vertices)
		{
			vertex.boneIndices = {0, 0, 0, 0};
			vertex.weights = {1.0f, 0.0f, 0.0f, 0.0f};
		}

		return;
	}

	assert(numSkin == 1);

	// Assume only one deformer
	FbxSkin* fbxSkin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
	int numCluster = fbxSkin->GetClusterCount();
	int numControlPoints = fbxMesh->GetControlPointsCount();
	vector<VertexWeight> tmpWeightList(numControlPoints);

	for (int i = 0; i < numCluster; i++)
	{
		FbxCluster* fbxCluster = fbxSkin->GetCluster(i);

		if (!fbxCluster->GetLink())
		{
			continue;
		}

		unsigned int boneIndex = model->skeleton->bones.size();

		assert(fbxCluster->GetLinkMode() == FbxCluster::eNormalize);

		// Read skeleton bones data (transformation matrix of each bone)
		string boneName(fbxCluster->GetLink()->GetName());

		if (!model->skeleton->FindBoneByName(boneName))
		{
			if (boneIndex >= MAXBONE)
			{
				PrintTab("Too many bones to load!!");
			}
			else
			{
				// Read weights of each vertex
				int numIndexInCluster = fbxCluster->GetControlPointIndicesCount();
				int* indicesInCluster = fbxCluster->GetControlPointIndices();
				double* weights = fbxCluster->GetControlPointWeights();

				for (int j = 0; j < numIndexInCluster; j++)
				{
					tmpWeightList[indicesInCluster[j]].AddBoneData(boneIndex, weights[j]);
				}

				// Normalize weights
				/*for (int inVert = 0; inVert < tmpWeightList.size(); inVert++)
				{
					tmpWeightList[inVert].Normalize();
				}*/

				// Read animation bone matrix
				FbxAMatrix fbxGlobalBoneBaseMatrix;// = fbxCluster->GetLink()->EvaluateGlobalTransform().Inverse().Transpose();

				FbxAMatrix referenceGlobalInitPosition;
				FbxAMatrix clusterGlobalInitPosition;

				fbxCluster->GetTransformMatrix(referenceGlobalInitPosition);
				fbxCluster->GetTransformLinkMatrix(clusterGlobalInitPosition);

				// To be considered when importing Maya fbx model
				FbxAMatrix geometricTransform = GetTransformMatrix(fbxCluster->GetLink());
				referenceGlobalInitPosition *= geometricTransform;

				fbxGlobalBoneBaseMatrix = clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition;

				// Store bone data
				Bone bone;
				bone.name = boneName;
				bone.boneIndex = boneIndex;
				bone.fbxNode = fbxCluster->GetLink();
				bone.globalBindposeInverseMatrix = fbxGlobalBoneBaseMatrix;

				model->skeleton->bones.push_back(bone);
			}
		}
	}

	// Deployed in the index
	for (unsigned int i = 0; i < mesh->numVertices; i++)
	{
		mesh->vertices[i].boneIndices.x = tmpWeightList[mesh->indices[i]].boneWeight[0].first;
		mesh->vertices[i].boneIndices.y = tmpWeightList[mesh->indices[i]].boneWeight[1].first;
		mesh->vertices[i].boneIndices.z = tmpWeightList[mesh->indices[i]].boneWeight[2].first;
		mesh->vertices[i].boneIndices.w = tmpWeightList[mesh->indices[i]].boneWeight[3].first;

		mesh->vertices[i].weights.x = tmpWeightList[mesh->indices[i]].boneWeight[0].second;
		mesh->vertices[i].weights.y = tmpWeightList[mesh->indices[i]].boneWeight[1].second;
		mesh->vertices[i].weights.z = tmpWeightList[mesh->indices[i]].boneWeight[2].second;
		mesh->vertices[i].weights.w = tmpWeightList[mesh->indices[i]].boneWeight[3].second;
	}
}

void MeshImporter::LoadMaterials(FbxNode* node, MeshEntry* mesh, ID3D11Device3* device,
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
						LoadTexture(model->path.c_str(), texture_name, mesh, isTextured, device, context);

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
					LoadTexture(model->path.c_str(), texture_name, mesh, isTextured, device, context);
					
					PrintTab(to_string(texture_count) + " Single texture loaded!");
				}
			}
		}
	}
}

