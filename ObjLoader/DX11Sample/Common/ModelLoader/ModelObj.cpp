#include "pch.h"
#include <ppltasks.h>
#include <string>

#include "ModelObj.h"

using namespace ModelImporter;
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

void ModelObj::MeshEntry::InitResources(ID3D11Device3* device)
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

void ModelObj::InitMesh(ID3D11Device3* device)
{
	PrintTab("Start init mesh");

	for (unsigned int i = 0; i < entries.size(); i++)
	{
		entries[i].InitResources(device);
	}

	PrintTab("End init mesh");
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


