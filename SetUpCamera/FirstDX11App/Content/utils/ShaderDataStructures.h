#pragma once

#include "pch.h"

using namespace DirectX;

namespace ShaderDataStructure
{
	struct ModelViewProjectionConstantBuffer
	{
		XMFLOAT4X4 model;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct Vertex
	{
		XMFLOAT3 pos;
		//XMFLOAT3 color;
	};
}

