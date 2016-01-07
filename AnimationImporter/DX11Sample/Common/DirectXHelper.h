#pragma once

#include <pch.h>
#include <ppltasks.h>	// For create_task

using namespace std;

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Function that reads from a binary file asynchronously.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([] (StorageFile^ file) 
		{
			return FileIO::ReadBufferAsync(file);
		}).then([] (Streams::IBuffer^ fileBuffer) -> std::vector<byte> 
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	inline void PrintTab(string str)
	{
		OutputDebugStringA(("\n" + str + "\n").c_str());
	}

	inline const wchar_t* GetWC(const char *c)
	{
		const size_t cSize = strlen(c) + 1;
		size_t outSize;
		wchar_t* wc = new wchar_t[cSize];

		mbstowcs_s(&outSize, wc, cSize, c, cSize - 1);

		return wc;
	}

	inline char* GetLower(const char* c)
	{
		char* out = new char[strlen(c) + 1];
		int temp = 'Z' - 'z';

		for (unsigned int i = 0; i < strlen(c) + 1; i++)
		{
			if (c[i] <= 'Z' && c[i] >= 'A')
			{
				out[i] = c[i] - temp;
			}
			else if (c[i] == '_')
			{
				out[i] = '-';
			}
			else
			{
				out[i] = c[i];
			}
		}

		return out;
	}

	// Put into Utilities file
	inline void LoadTexture(const char* fileName, ID3D11Device3* device,
		ID3D11DeviceContext3* context, ID3D11ShaderResourceView* srv)
	{
		/*PrintTab("Start load texture file");
		PrintTab(fileName);
		
		//string path = "Assets\\starwars-millennium-falcon\\";
		string path("Assets\\Wooden_House\\");
		string fileNameStr(fileName);

		if (fileNameStr.find(".fbx") == -1 && fileNameStr.find(".FBX") == -1)
		{
			fileNameStr += ".fbx";
		}

		HRESULT hr = CreateWICTextureFromFile(device, context, GetWC((path + fileNameStr).c_str()),
			nullptr, &srv);

		if (FAILED(hr))
		{
			// Try both uppercase and lowercase
			HRESULT hr2 = CreateWICTextureFromFile(device, context, 
				GetWC((path + GetLower(fileNameStr.c_str)).c_str()),
				nullptr, &srv);

			if (FAILED(hr2))
			{
				// Set a breakpoint on this line to catch Win32 API errors.
				throw Platform::Exception::CreateException(hr);
			}
		}

		PrintTab("End load texture file");*/
	}

#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
			);

		return SUCCEEDED(hr);
	}
#endif
}
