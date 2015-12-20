#pragma once

namespace DX11Sample
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct Vertex
	{
		/*Vertex(float x, float y, float z)
		{
			pos.x = x;
			pos.y = y;
			pos.z = z;
		}

		Vertex(DirectX::XMFLOAT3 inputPos)
		{
			pos = inputPos;
		}*/

		DirectX::XMFLOAT3 pos;
		//XMFLOAT3 color;
	};
}