#pragma once

#include <ppltasks.h>

using namespace Platform;
using namespace Windows::Storage;
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

	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring)
	{
		//auto folder = Windows::ApplicationModel::P
	}
}
