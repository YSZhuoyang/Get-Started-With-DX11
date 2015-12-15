#pragma once

#include "Content\utils\ShaderDataStructures.h"

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Platform;
using namespace DirectX;
using namespace ShaderDataStructure;


ref class Game sealed
{
private:
	ComPtr<ID3D11Device1> dev;
	ComPtr<ID3D11DeviceContext1> devCon;
	ComPtr<IDXGISwapChain1> swapChain;
	ComPtr<ID3D11RenderTargetView> renderTarget;
	ComPtr<ID3D11DepthStencilView> depthStencilView;

	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> constantBuffer;

	ModelViewProjectionConstantBuffer MVPMatrix;

	ComPtr<ID3D11InputLayout> inputLayout;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	float windowWidth;
	float windowHeight;

	bool msaa4xEnabled;

public:
	void Initialize();
	void InitData();
	void InitPipeline();
	void SetupSwapChainAndDepthBuffers();
	void SetupViewPort();
	void Update();
	void Render();
	void SetupCamera();
};

