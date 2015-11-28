#pragma once

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Platform;
using namespace DirectX;


struct VERTEX
{
	float X, Y, Z;    // vertex position
};

ref class Game sealed
{
private:
	ComPtr<ID3D11Device1> dev;
	ComPtr<ID3D11DeviceContext1> devCon;
	ComPtr<IDXGISwapChain1> swapChain;
	ComPtr<ID3D11RenderTargetView> renderTarget;
	ComPtr<ID3D11Buffer> vertexBuffer;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

public:
	void Initialize();
	void SetupBuffer();
	void InitPipeline();
	void Update();
	void Render();
};

