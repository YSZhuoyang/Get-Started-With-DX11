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

	// Animation transform data
	struct AnimationConstantBuffer
	{
		XMFLOAT4X4 meshBoneMatrices[MAXBONE];
	};

	// Used to send per-vertex data to the vertex shader.
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		XMUINT4 boneIndices;
		XMFLOAT4 weights;
	};
}