#include "pch.h"
#include "Game.h"
#include "Content\utils\ShaderFileLoader.h"
#include "Content\utils\Helper.h"
#include <fstream>

using namespace ShaderLoader;
using namespace Windows::UI::Popups;
using namespace Windows::UI::Xaml;
using namespace DXHelper;


void Game::Initialize()
{
	// Enable 4x msaa
	msaa4xEnabled = false;

	CoreWindow^ window = CoreWindow::GetForCurrentThread();

	// Get window size
	windowWidth = window->Bounds.Width;
	windowHeight = window->Bounds.Height;

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
	
	SetupSwapChainAndDepthBuffers();
	SetupViewPort();
	SetupCamera();
	InitData();
	InitPipeline();
}

void Game::SetupSwapChainAndDepthBuffers()
{
	// Check msaa support
	UINT m4mxMsaaQuality;

	dev->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4mxMsaaQuality);

	if (m4mxMsaaQuality > 0)
	{
		//msaa4xEnabled = true;
	}
	else
	{
		//MessageDialog dialog("Msaa is not supported!", "Notice!");
		//dialog.ShowAsync();

		//msaa4xEnabled = false;
	}

	// Set up swap chain
	DXGI_SWAP_CHAIN_DESC1 scd = { 0 };	// Initialize everything to 0
	scd.Width = lround(convertDipToPixels(windowWidth));
	scd.Width = lround(convertDipToPixels(windowHeight));
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2; // 1 for double buffering, 2 for triple buffering
	scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // DXGI_SWAP_EFFECT_DISCARD
	
	if (msaa4xEnabled)
	{
		scd.SampleDesc.Count = 4;
		scd.SampleDesc.Quality = m4mxMsaaQuality - 1;
	}
	else
	{
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
	}

	// Obtain Factory
	ComPtr<IDXGIDevice1> idxgiDevice;
	dev.As(&idxgiDevice);

	ComPtr<IDXGIAdapter> idxgiAdapter;
	idxgiDevice->GetAdapter(&idxgiAdapter);

	ComPtr<IDXGIFactory2> idxgiFactory;
	idxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &idxgiFactory);

	CoreWindow^ window = CoreWindow::GetForCurrentThread();

	idxgiFactory->CreateSwapChainForCoreWindow(
		dev.Get(),
		reinterpret_cast<IUnknown*> (window),
		&scd,
		nullptr,
		&swapChain);

	// Set up back buffer and render target view
	ComPtr<ID3D11Texture2D> backBuffer;

	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
	
	dev->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		&renderTarget);

	// Init depth buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = lround(convertDipToPixels(windowWidth));
	depthStencilDesc.Width = lround(convertDipToPixels(windowHeight));
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	if (msaa4xEnabled)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4mxMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthStencilBuffer;

	dev->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		&depthStencilBuffer);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

	dev->CreateDepthStencilView(
		depthStencilBuffer.Get(),
		&depthStencilViewDesc,
		&depthStencilView);
}

void Game::SetupViewPort()
{
	D3D11_VIEWPORT viewport = { 0 };

	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = windowWidth;
	viewport.Height = windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	devCon->RSSetViewports(1, &viewport);
}

void Game::InitData()
{
	// Init vertex data and buffer
	Vertex vertices[] = {
		XMFLOAT3(0.0f, 0.5f, 0.0f),
		XMFLOAT3(0.45f, -0.5f, 0.0f),
		XMFLOAT3(-0.45f, -0.5f, 0.0f)
	};

	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd = {vertices, 0, 0};
	dev->CreateBuffer(&bd, &srd, &vertexBuffer);

	// Initialize constant buffer for storing mvp matrix
	CD3D11_BUFFER_DESC constantBufferDesc(
		sizeof(ModelViewProjectionConstantBuffer),
		D3D11_BIND_CONSTANT_BUFFER);

	dev->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&constantBuffer);
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
}

void Game::Update()
{

}

void Game::Render()
{
	// Set render target
	//devCon->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);
	devCon->OMSetRenderTargets(1, renderTarget.GetAddressOf(), depthStencilView.Get());

	// Clear backbuffer
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devCon->ClearRenderTargetView(renderTarget.Get(), color);

	// Update camera data
	devCon->UpdateSubresource1(
		constantBuffer.Get(),
		0,
		NULL,
		&MVPMatrix,
		0,
		0,
		0);

	// Set buffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	devCon->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	devCon->IASetInputLayout(inputLayout.Get());

	devCon->VSSetConstantBuffers1(
		0,
		1,
		constantBuffer.GetAddressOf(),
		nullptr,
		nullptr);

	// Set primitive topology and draw
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devCon->Draw(3, 0);

	swapChain->Present(1, 0);
}

void Game::SetupCamera()
{
	float aspectRatio = windowWidth / windowHeight;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f);

	XMStoreFloat4x4(
		&MVPMatrix.projection,
		XMMatrixTranspose(perspectiveMatrix));

	// Construct a view matrix
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&MVPMatrix.view,
		XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	XMStoreFloat4x4(&MVPMatrix.model,
		XMMatrixIdentity());
}
