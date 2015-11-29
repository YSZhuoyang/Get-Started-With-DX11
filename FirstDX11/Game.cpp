#include "pch.h"
#include "Game.h"
#include "Content\utils\ShaderFileLoader.h"
#include <fstream>

using namespace ShaderLoader;


void Game::Initialize()
{
	// Create device
	ComPtr<ID3D11Device> dev11;
	ComPtr<ID3D11DeviceContext> devCon11;

	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&dev11,
		nullptr,
		&devCon11);

	dev11.As(&dev);
	devCon11.As(&devCon);

	// Obtain Factory
	ComPtr<IDXGIDevice1> idxgiDevice;
	dev.As(&idxgiDevice);

	ComPtr<IDXGIAdapter> idxgiAdapter;
	idxgiDevice->GetAdapter(&idxgiAdapter);

	ComPtr<IDXGIFactory2> idxgiFactory;
	idxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &idxgiFactory);

	// Set up swap chain
	DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	scd.SampleDesc.Count = 1;

	CoreWindow^ window = CoreWindow::GetForCurrentThread();

	idxgiFactory->CreateSwapChainForCoreWindow(
		dev.Get(), 
		reinterpret_cast<IUnknown*> (window), 
		&scd, 
		nullptr, 
		&swapChain);

	SetupBuffer();
	InitPipeline();

	// Setup view port
	D3D11_VIEWPORT viewport = { 0 };

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = window->Bounds.Width;
	viewport.Height = window->Bounds.Height;

	devCon->RSSetViewports(1, &viewport);
}

void Game::SetupBuffer()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);

	dev->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTarget);

	// Init vertex data
	VERTEX vertices[] =
	{
		{ 0.0f, 0.5f, 0.0f },
		{ 0.45f, -0.5f, 0.0f },
		{ -0.45f, -0.5f, 0.0f },
	};

	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(VERTEX) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd = {vertices, 0, 0};
	dev->CreateBuffer(&bd, &srd, &vertexBuffer);
}

void Game::InitPipeline()
{
	// Load shader files
	Array<byte>^ VSFile = LoadShaderFile("VertexShader.cso");
	Array<byte>^ PSFile = LoadShaderFile("PixelShader.cso");

	// Init shaders
	dev->CreateVertexShader(VSFile->Data, VSFile->Length, nullptr, &vertexShader);
	dev->CreatePixelShader(PSFile->Data, PSFile->Length, nullptr, &pixelShader);

	// Set shaders
	devCon->VSSetShader(vertexShader.Get(), nullptr, 0);
	devCon->PSSetShader(pixelShader.Get(), nullptr, 0);

	// Init input layout
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create and set the input layout
	dev->CreateInputLayout(ied, ARRAYSIZE(ied), VSFile->Data, VSFile->Length, &inputLayout);
	devCon->IASetInputLayout(inputLayout.Get());
}

void Game::Update()
{

}

void Game::Render()
{
	// Set render target
	devCon->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);

	// Clear backbuffer
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devCon->ClearRenderTargetView(renderTarget.Get(), color);

	// Set buffers
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devCon->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set primitive topology
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Drawing
	devCon->Draw(3, 0);

	swapChain->Present(1, 0);
}


