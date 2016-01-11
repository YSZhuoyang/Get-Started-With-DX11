#include "pch.h"
#include <ppltasks.h>
#include <string>

#include "ModelObj.h"
#include "Common\Utilities.h"


using namespace ModelImporter;
using namespace DirectX;
using namespace DX;
using namespace Utilities;
using namespace std;
using namespace Windows;

ModelObj::ModelObj() :
	triangulated(true)
{
	//device = m_deviceResources->GetD3DDevice();
	//context = m_deviceResources->GetD3DDeviceContext();

	skeleton = new Skeleton();
}

void VertexWeight::AddBoneData(unsigned int index, float weight)
{
	for (int i = 0; i < MAXBONEPERVERTEX; i++)
	{
		// Add data in descending order
		if (boneWeight[i].second < weight)
		{
			for (int j = MAXBONEPERVERTEX - 2; j >= i; j--)
			{
				//boneWeight[j + 1] = boneWeight[j];

				boneWeight[j + 1].first = boneWeight[j].first;
				boneWeight[j + 1].second = boneWeight[j].second;
			}

			boneWeight[i].first = index;
			boneWeight[i].second = weight;

			break;
		}
	}
}

void VertexWeight::Normalize()
{
	float total = 0.0f;

	for (int i = 0; i < MAXBONEPERVERTEX; i++)
	{
		if (boneWeight[i].second != 0.0f)
		{
			total += boneWeight[i].second;
		}
	}

	if (total != 0.0f)
	{
		for (int i = 0; i < MAXBONEPERVERTEX; i++)
		{
			boneWeight[i].second /= total;
		}
	}
}

Bone* Skeleton::FindBoneByName(string boneName)
{
	for (vector<Bone>::iterator it = bones.begin(); it != bones.end(); ++it)
	{
		if (it->name.compare(boneName) == 0)
		//if (strcmp(it->name.c_str(), boneName.c_str()) == 0)
		{
			return &(*it);
		}
	}

	//PrintTab("Bone not found!");

	return nullptr;
}

XMFLOAT4X4 Bone::GetBoneMatrix(unsigned int frame)
{
	FbxTime time;
	//PrintTab("Frame: " + to_string(frame));
	time.Set(FbxTime::GetOneFrameValue(FbxTime::eFrames60) * (frame % 60));
	//time.SetFrame(5 % 60, FbxTime::eFrames60);

	FbxAMatrix fbxCurrMatrix = fbxNode->EvaluateGlobalTransform(time);
	XMFLOAT4X4 currMatrix;

	/*for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
		{
			currMatrix.m[r][c] = (float)fbxCurrMatrix.mData[r][c];

			PrintTab("Global bone mat: " + to_string(currMatrix.m[r][c]));
		}*/

	ConvertFbxAMatrixToDXMatrix(&currMatrix, fbxCurrMatrix);

	XMMATRIX matrix = XMMatrixMultiply(XMLoadFloat4x4(&currMatrix), XMLoadFloat4x4(&globalBoneBaseMatrix));
	XMFLOAT4X4 outMatrix;

	XMStoreFloat4x4(&outMatrix, matrix);

	return outMatrix;
}

void MeshEntry::InitResources(ID3D11Device3* device)
{
	PrintTab("Start init resources of a mesh");

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
	/*D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
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
	*/

	PrintTab("End init resources of a mesh");
}

XMFLOAT4X4 MeshEntry::GetMeshMatrix(float frame)
{
	FbxTime time;
	
	time.Set(FbxTime::GetOneFrameValue(FbxTime::eFrames60) * frame);

	FbxAMatrix fbxCurrMatrix = fbxNode->EvaluateGlobalTransform(time);
	XMFLOAT4X4 currMatrix;

	/*for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
		{
			currMatrix.m[r][c] = (float)fbxCurrMatrix.mData[r][c];

			PrintTab("Global mesh mat: " + to_string(currMatrix.m[r][c]));
		}*/
	
	ConvertFbxAMatrixToDXMatrix(&currMatrix, fbxCurrMatrix);

	XMMATRIX matrix = XMMatrixMultiply(XMLoadFloat4x4(&currMatrix), XMLoadFloat4x4(&globalMeshBaseMatrix));
	XMFLOAT4X4 outMatrix;

	XMStoreFloat4x4(&outMatrix, matrix);

	return outMatrix;
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

void ModelObj::InitAnimationData(ID3D11Device3* device)
{
	// Only bone matrix loaded
	for (unsigned short i = 0; i < MAXBONE; i++)
	{
		if (i < skeleton->bones.size())
		{
			//XMMatrixMultiply(StoreMatrix);
			//animMatrixBufferData.meshBoneMatrices[i] = globalRootTransform * skeleton->bones[i].globalBoneBaseMatrix;
			animMatrixBufferData.meshBoneMatrices[i] = skeleton->bones[i].globalBoneBaseMatrix;

			//PrintTab("Bone Mat: " + to_string(animMatrixBufferData.meshBoneMatrices[i].m[0][0]));
		}
		else
		{
			XMStoreFloat4x4(&animMatrixBufferData.meshBoneMatrices[i], XMMatrixIdentity());
		}

		//XMStoreFloat4x4(&animMatrixBufferData.meshBoneMatrices[i], XMMatrixIdentity());
	}

	// Put into the block of a asyn-task
	// Create animation constant buffer
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(AnimationConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	//constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	
	DX::ThrowIfFailed(
		device->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			&animConstantBuffer
			)
		);
}

void ModelObj::Render(ID3D11DeviceContext3* context, ID3D11SamplerState* sampleState)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (vector<MeshEntry>::iterator mesh = entries.begin(); mesh != entries.end(); ++mesh)
	{
		// Update constant buffer data
		context->UpdateSubresource1(
			animConstantBuffer.Get(),
			0,
			NULL,
			&animMatrixBufferData,
			0,
			0,
			0
			);

		// Bind constant buffer for animations
		context->VSSetConstantBuffers1(
			1,// change
			1,
			animConstantBuffer.GetAddressOf(),
			nullptr,
			nullptr
			);

		// Bind vertex buffer
		context->IASetVertexBuffers(
			0,
			1,
			mesh->vertexBuffer.GetAddressOf(),
			&stride,
			&offset
			);

		/*context->IASetIndexBuffer(
			mesh->indexBuffer.Get(),
			DXGI_FORMAT_R32_UINT, // Each index is one 32-bit unsigned integer (short).
			0
			);*/

		// Set the sampler state in the pixel shader.
		context->PSSetSamplers(0, 1, &sampleState);
		context->PSSetShaderResources(0, 1, mesh->srv.GetAddressOf());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		//context->IASetInputLayout(mesh->inputLayout.Get());

		context->Draw(mesh->numVertices, 0);
		/*context->DrawIndexed(
			mesh->numIndices,
			0,
			0);*/
	}
}

void ModelObj::Update(StepTimer const& timer)
{
	for (int i = 0; i < skeleton->bones.size(); i++)
	{
		XMFLOAT4X4 currMat = skeleton->bones[i].GetBoneMatrix(timer.GetFrameCount());
		XMMATRIX boneMeshMatrix = XMMatrixMultiply(
			XMMatrixTranspose(XMLoadFloat4x4(&skeleton->bones[i].globalBoneBaseMatrix)), 
			XMMatrixTranspose(XMLoadFloat4x4(&currMat))
			);

		XMStoreFloat4x4(
			&animMatrixBufferData.meshBoneMatrices[skeleton->bones[i].boneIndex], 
			boneMeshMatrix
			);
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
	
	string typeName = GetAttributeTypeName(attr->GetAttributeType());
	string attrName = attr->GetName();

	OutputDebugStringA(("\nAttr type: " + to_string(attr->GetAttributeType()) + "\n").c_str());
	OutputDebugStringA(("Attr name: " + attrName + "\n").c_str());
}

FbxString ModelObj::GetAttributeTypeName(FbxNodeAttribute::EType type)
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
	for (vector<MeshEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
	{
		it->vertexBuffer.Reset();
		it->srv.Reset();
	}

	animConstantBuffer.Reset();
}


