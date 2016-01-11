#pragma once
#include "Content\ShaderStructures.h"

using namespace DirectX;
using namespace DX11Sample;

namespace Utilities
{
	class Camera
	{
	public:
		Camera();
		~Camera();

		XMVECTORF32 eye = { 0.0f, 14.0f, 60.0f, 0.0f };
		XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		ViewProjectionConstantBuffer VPMatrixData;
	};
}


