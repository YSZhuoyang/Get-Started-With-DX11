#pragma once
#include "Common\DeviceResources.h"

using namespace DX;
using namespace DirectX;

namespace DX11Sample
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		XMFLOAT4X4 model;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct AnimationConstantBuffer
	{
		XMFLOAT4X4 meshBoneMatrices[MAXBONE];
	};

	/*struct MeshAnimationConstantBuffer
	{
		XMFLOAT4X4 meshMatrix;
	};*/

	// Used to send per-vertex data to the vertex shader.
	/*struct VertexPositionColor
	{
		XMFLOAT3 pos;
		XMFLOAT3 color;
	};*/

	struct Vertex
	{
		/*Vertex(float x, float y, float z)
		{
			pos.x = x;
			pos.y = y;
			pos.z = z;
		}*/

		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		
		//vector<BoneWeight> weight;
		XMUINT4 boneIndices;
		XMFLOAT4 weights;
		//BoneWeight weight[4];
	};
}