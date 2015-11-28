#include "pch.h"
#include "Game.h"


Game::Game()
{
}

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
}

void Game::Update()
{

}

void Game::Render()
{
	swapChain->Present(1, 0);
}


