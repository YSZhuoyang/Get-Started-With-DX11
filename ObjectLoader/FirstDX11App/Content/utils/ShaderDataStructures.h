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
		Vertex(float x, float y, float z)
		{
			pos.x = x;
			pos.y = y;
			pos.z = z;
		}

		XMFLOAT3 pos;
		//XMFLOAT3 color;
	};
}

