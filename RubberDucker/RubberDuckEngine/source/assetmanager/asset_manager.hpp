#pragma once
#include "vulkan/mesh.hpp"
#include "vulkan/texture.hpp"
#include "vulkan/texture_data.hpp"
#include <stbi/stb_image.h>

namespace RDE
{

class AssetManager
{
  public:
    void loadModel(const char* modelPath);
    void loadModels(const char* folderPath);
    void loadTexture(const char* texturePath);
    void loadTextures(const char* folderPath);

    [[nodiscard]] __forceinline const char* getAssetName(uint32_t id)
    {
        return m_assetNames[id].c_str();
    }
    [[nodiscard]] __forceinline uint32_t getAssetID(const char* assetName)
    {
        return m_assetIDs[assetName];
    };
    [[nodiscard]] __forceinline uint32_t assetCount()
    {
        return static_cast<uint32_t>(m_assetIDs.size());
    };

    [[nodiscard]] __forceinline Vulkan::Mesh& getMesh(uint32_t id)
    {
        return m_meshes[id];
    };

    [[nodiscard]] __forceinline Vulkan::Mesh& getMesh(const char* assetName)
    {
        return m_meshes[m_assetIDs[assetName]];
    };

    [[nodiscard]] __forceinline uint32_t meshCount()
    {
        return static_cast<uint32_t>(m_meshes.size());
    };

    [[nodiscard]] __forceinline Vulkan::Texture& getTexture(uint32_t id)
    {
        return m_textures[id];
    };

    [[nodiscard]] __forceinline Vulkan::Texture&
    getTexture(const char* assetName)
    {
        return m_textures[m_assetIDs[assetName]];
    };

    [[nodiscard]] __forceinline uint32_t textureCount()
    {
        return static_cast<uint32_t>(m_textures.size());
    };

    template <typename TCallable> void eachMesh(TCallable&& callable)
    {
        if constexpr (std::is_invocable_v<TCallable, Vulkan::Mesh&>) {
            for (auto& [id, mesh] : m_meshes) {
                callable(mesh);
            }
        } else if constexpr (std::is_invocable_v<TCallable, uint32_t,
                                                 Vulkan::Mesh&>) {
            for (auto& [id, mesh] : m_meshes) {
                callable(id, mesh);
            }
        } else if constexpr (std::is_invocable_v<TCallable, Vulkan::Mesh&,
                                                 uint32_t>) {
            for (auto& [id, mesh] : m_meshes) {
                callable(mesh, id);
            }
        }
    }

    template <typename TCallable> void eachTexture(TCallable&& callable)
    {
        if constexpr (std::is_invocable_v<TCallable, Vulkan::Texture&>) {
            for (auto& [id, texture] : m_textures) {
                callable(texture);
            }
        } else if constexpr (std::is_invocable_v<TCallable, uint32_t,
                                                 Vulkan::Texture&>) {
            for (auto& [id, texture] : m_textures) {
                callable(id, texture);
            }
        } else if constexpr (std::is_invocable_v<TCallable, Vulkan::Texture&,
                                                 uint32_t>) {
            for (auto& [id, texture] : m_textures) {
                callable(texture, id);
            }
        }
    }

  private:
    // GUIDs and Filenames
    std::unordered_map<uint32_t, std::string> m_assetNames;
    std::unordered_map<std::string, uint32_t> m_assetIDs;

    // Actual assets
    std::unordered_map<uint32_t, Vulkan::Mesh> m_meshes;
    std::unordered_map<uint32_t, Vulkan::Texture> m_textures;

    static constexpr uint32_t s_nullIndex =
        std::numeric_limits<uint32_t>::max();
};
} // namespace RDE
