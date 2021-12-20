#pragma once

namespace RDE
{
	class FileIO
	{
	public:
		using FileBufferType = std::vector<char>;

		static FileBufferType read(const char* filename);
		static FileBufferType read(std::string_view filename);
		static void write(const char* filename);
	};
}