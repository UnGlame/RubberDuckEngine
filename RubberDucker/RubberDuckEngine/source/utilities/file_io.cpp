#include "pch.hpp"
#include "file_io.hpp"
#include <fstream>

namespace RDE
{
	FileIO::FileBufferType RDE::FileIO::read(const char* filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error(fmt::format("Failed to open {}!", filename));
		}

		// Get file size from read position
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		// Go back to beginning and read all bytes into buffer
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	FileIO::FileBufferType RDE::FileIO::read(std::string_view filename)
	{
		return read(filename.data());
	}

	void RDE::FileIO::write(const char* filename)
	{
	}
}