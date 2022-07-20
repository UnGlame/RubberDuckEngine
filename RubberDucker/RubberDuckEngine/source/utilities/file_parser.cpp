#include "precompiled/pch.hpp"
#include "utilities/file_parser.hpp"

namespace RDE
{

FileParser::FileBufferType RDE::FileParser::read(const char *filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        RDE_LOG_CRITICAL(fmt::format("Failed to open {}!", filename));
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

FileParser::FileBufferType RDE::FileParser::read(std::string_view filename) {
    return read(filename.data());
}

void RDE::FileParser::write(const char *filename) {}
} // namespace RDE
