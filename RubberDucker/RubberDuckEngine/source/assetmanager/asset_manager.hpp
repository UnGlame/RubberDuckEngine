#pragma once
#include <stbi/stb_image.h>
#include "vulkan/mesh.hpp"

namespace RDE {

	class AssetManager
	{
	public:
		void loadModel(const char* modelPath);
		void loadModels(const char* folderPath);
		stbi_uc* loadTexture(const char* texturePath, int& texWidth, int& texHeight, int& texChannels);
		void freeTexture(stbi_uc* loadedTexture);

		[[nodiscard]] inline const char* getAssetName(uint32_t id)			{ return m_assetNames[id].c_str(); }
		[[nodiscard]] inline uint32_t getAssetID(const char* assetName)		{ return m_assetIDs[assetName]; };
		[[nodiscard]] inline Vulkan::Mesh& getMesh(uint32_t id)				{ return m_meshes[id]; };
		[[nodiscard]] inline Vulkan::Mesh& getMesh(const char* assetName)	{ return getMesh(m_assetIDs[assetName]); };

		template <typename TCallable>
		void eachMesh(TCallable&& callable)
		{
			if constexpr (std::is_invocable_v<TCallable, Vulkan::Mesh&>) {
				for (auto& [id, mesh] : m_meshes) {
					callable(mesh);
				}
			}
			else if constexpr (std::is_invocable_v<TCallable, uint32_t, Vulkan::Mesh&>) {
				for (auto& [id, mesh] : m_meshes) {
					callable(id, mesh);
				}
			}
			else /*(std::is_invocable_v<TCallable, Vulkan::Mesh&, uint32_t>)*/ {
				for (auto& [id, mesh] : m_meshes) {
					callable(mesh, id);
				}
			}
		}

	private:
		// GUIDs and Filenames
		std::unordered_map<uint32_t, std::string> m_assetNames;
		std::unordered_map<std::string, uint32_t> m_assetIDs;

		// Actual assets
		std::unordered_map<uint32_t, Vulkan::Mesh> m_meshes;

		static constexpr uint32_t s_nullIndex = std::numeric_limits<uint32_t>::max();
	};
}