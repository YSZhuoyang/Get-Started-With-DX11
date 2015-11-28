#include <fstream>
#include "pch.h"

using namespace std;

namespace ShaderLoader
{
	inline Array<byte>^ LoadShaderFile(string file)
	{
		Array<byte>^ fileData = nullptr;

		// Open file
		ifstream VertexFile(file, ios::in | ios::binary | ios::ate);

		// If file is opened successfully
		if (VertexFile.is_open())
		{
			int length = (int)VertexFile.tellg();

			fileData = ref new Array<byte>(length);
			VertexFile.seekg(0, ios::beg);
			VertexFile.read(reinterpret_cast<char*>(fileData->Data), length);
			VertexFile.close();
		}

		return fileData;
	}
}
