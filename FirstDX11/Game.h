#pragma once

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace DirectX;


ref class Game sealed
{
private:
	ComPtr<ID3D11Device1> dev;
	ComPtr<ID3D11DeviceContext1> devCon;
	ComPtr<IDXGISwapChain1> swapChain;

public:
	Game();
	void Initialize();
	void Update();
	void Render();
};

