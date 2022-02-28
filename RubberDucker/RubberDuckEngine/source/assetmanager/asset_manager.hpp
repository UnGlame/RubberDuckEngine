#pragma once
#include "vulkan/mesh.hpp"
#include <stbi/stb_image.h>

namespace RDE {

	class AssetManager
	{
	public:
		Vulkan::Mesh loadModel(const char* modelPath);
		stbi_uc* loadTexture(const char* texturePath, int& texWidth, int& texHeight, int& texChannels);
		void freeTexture(stbi_uc* loadedTexture);

		const char* getAssetName(uint32_t id);
		uint32_t getAssetID(const char* assetName);

	private:
		std::unordered_map<uint32_t, const char*> m_assetNames;
		std::unordered_map<const char*, uint32_t> m_assetIDs;
	};
}