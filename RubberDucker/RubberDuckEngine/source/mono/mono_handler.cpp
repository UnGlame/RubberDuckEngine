#include "precompiled/pch.hpp"
#include "mono_handler.hpp"

namespace RDE
{
void MonoHandler::init()
{
    /*RDELOG_INFO("Initialising Mono");

#ifdef RDE_DEBUG
    std::string directory =
        std::filesystem::current_path().string() + "\\dep\\mono\\lib\\debug";
#else
    std::string directory =
        std::filesystem::current_path().string() + "\\dep\\mono\\lib\\release";

#endif

    mono_set_dirs(directory.c_str(), "");
    m_root = mono_jit_init_version("RDEMono", "v4.0.30319");

    if (!m_root)
        RDELOG_ERROR("Failed to create domain");

    GenerateDLL();
    LoadDLLImage("RDEScriptsAPI.dll", m_APIImage, m_APIAssembly);*/
}

void MonoHandler::cleanup()
{ /*mono_jit_cleanup(m_root);*/
}

void MonoHandler::GenerateDLL()
{
    RDELOG_INFO("Generating DLL Images");

    const auto currentPath = std::filesystem::current_path().string();
    std::filesystem::create_directory(currentPath + "//dep//mono//bin");
    std::string commandPath = "cmd /C  "
                              "" +
                              currentPath + "//dep//mono//bin//";
    system((commandPath + "mcs_api.bat"
                          "")
               .c_str());
    system((commandPath + "mcs_scripts.bat"
                          "")
               .c_str());
}

bool MonoHandler::LoadDLLImage(const char* filename, MonoImage*& image,
                               MonoAssembly*& assembly)
{
    RDELOG_INFO("Loading DLL Images");
    char* arr = nullptr;
    uint32_t len = 0;
    std::ifstream file(filename, std::ifstream::binary);

    if (!file) {
        RDELOG_ERROR("Failed to load DLL: DLL cannot be found!");
        return false;
    }

    // get length of file:
    file.seekg(0, file.end);
    auto length = file.tellg();
    len = static_cast<uint32_t>(length);
    file.seekg(0, file.beg);

    arr = new char[length];
    file.read(arr, length);
    file.close();

    MonoImageOpenStatus status;
    image = nullptr;
    image = mono_image_open_from_data_with_name(arr, len, true /* copy data */,
                                                &status, false /* ref only */,
                                                filename);
    bool result = true;
    if (image)
        assembly =
            mono_assembly_load_from_full(image, filename, &status, false);
    else
        result = false;

    delete[] arr;
    return result;
}
} // namespace RDE
