#include "precompiled/pch.hpp"
#include "mono_handler.hpp"

namespace RDE
{
	void MonoHandler::init()
	{
		RDE_LOG_INFO("Initialising Mono");

		mono_set_dirs((std::filesystem::current_path().string() + "\\dep\\mono\\lib\\").c_str(), "");
		m_root = mono_jit_init_version("RDEMono", "v4.0.30319");
		
		if (!m_root)
			RDE_LOG_ERROR("Failed to create domain");

		GenerateDLL();
		LoadDLLImage("RDEScriptsAPI.dll", m_APIImage, m_APIAssembly);
	}

	void MonoHandler::cleanup()
	{
		mono_jit_cleanup(m_root);
	}

	void MonoHandler::GenerateDLL()
	{
		RDE_LOG_INFO("Generating DLL Images");
		std::string commandPath = "cmd /C  """ + std::filesystem::current_path().string() + "//dep//mono//bin//";
		system((commandPath + "mcs_api.bat""").c_str());
		system((commandPath + "mcs_scripts.bat""").c_str());
	}

	bool MonoHandler::LoadDLLImage(const char* filename, MonoImage*& image, MonoAssembly*& assembly)
	{
		RDE_LOG_INFO("Loading DLL Images");
		char* arr = nullptr;
		uint32_t len = 0;
		std::ifstream file(filename, std::ifstream::binary);

		if (!file) {
			RDE_LOG_ERROR("Failed to load DLL: DLL cannot be found!");
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
		image = mono_image_open_from_data_with_name(arr, len, true /* copy data */, &status, false /* ref only */, filename);
		bool result = true;
		if (image)
			assembly = mono_assembly_load_from_full(image, filename, &status, false);
		else
			result = false;

		delete[] arr;
		return result;
	}
}