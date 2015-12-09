#pragma once

#include <ppltasks.h>

using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Graphics::Display;
using namespace Concurrency;

namespace DXHelper
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw Exception::CreateException(hr);
		}
	}

	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring filename)
	{
		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(StringReference(filename.c_str()))).then([] (StorageFile^ file)
		{
			return FileIO::ReadBufferAsync(file);
		}).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
		{
			std::vector<byte> returnBuffer;

			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));

			return returnBuffer;
		});
	}

	inline float convertDipToPixels(float dips)
	{
		static const float dipsPerInch = 96.0f;

		return floor(dips * DisplayProperties::LogicalDpi / dipsPerInch + 0.5f);
	}
}
