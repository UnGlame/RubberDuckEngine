#pragma once
#include "vulkan/data_types/mesh.hpp"
#include "vulkan/data_types/texture.hpp"

#include <stbi/stb_image.h>

namespace RDE {

class AssetManager
{
public:
    void loadModel(const char* modelPath);
    void loadModels(const char* folderPath = k_modelDirPath);
    void loadTexture(const char* texturePath);
    void loadTextures(const char* folderPath = k_textureDirPath);

    [[nodiscard]] std::string getFileName(const char* filePath) const;
    [[nodiscard]] std::string joinPath(const char* assetDir, const char* filename) const;
    [[nodiscard]] const char* getAssetPath(uint32_t id) const;
    [[nodiscard]] std::string getAssetName(uint32_t id) const;
    [[nodiscard]] uint32_t getAssetId(const char* assetPath) const;
    [[nodiscard]] uint32_t getModelId(const char* modelName, const char* modelDir = k_modelDirPath) const;
    [[nodiscard]] uint32_t getTextureId(const char* textureName, const char* textureDir = k_textureDirPath) const;
    [[nodiscard]] uint32_t assetCount() const;
    [[nodiscard]] Vulkan::Mesh& getMesh(uint32_t id);
    [[nodiscard]] Vulkan::Mesh& getMesh(const char* assetName);
    [[nodiscard]] uint32_t meshCount() const;
    [[nodiscard]] Vulkan::Texture& getTexture(uint32_t id);
    [[nodiscard]] Vulkan::Texture& getTexture(const char* assetName);
    [[nodiscard]] uint32_t textureCount() const;
    [[nodiscard]] std::vector<std::string> getAssetNames() const;
    [[nodiscard]] std::vector<std::string> getAssetPaths() const;
    [[nodiscard]] std::vector<std::string> getModelPaths() const;
    [[nodiscard]] std::vector<std::string> getTexturePaths() const;
    [[nodiscard]] std::vector<std::string> getModelNames() const;
    [[nodiscard]] std::vector<std::string> getTextureNames() const;

    template<typename TCallable>
    void eachMesh(TCallable&& callable)
    {
        if constexpr (std::is_invocable_v<TCallable, Vulkan::Mesh&>) {
            for (auto& [id, mesh] : m_meshes) {
                RDELOG_INFO("Each mesh: id = {}, name = {}", id, getAssetName(id));
                callable(mesh);
            }
        } else if constexpr (std::is_invocable_v<TCallable, uint32_t, Vulkan::Mesh&>) {
            for (auto& [id, mesh] : m_meshes) {
                callable(id, mesh);
            }
        } else if constexpr (std::is_invocable_v<TCallable, Vulkan::Mesh&, uint32_t>) {
            for (auto& [id, mesh] : m_meshes) {
                callable(mesh, id);
            }
        }
    }

    template<typename TCallable>
    void eachTexture(TCallable&& callable)
    {
        if constexpr (std::is_invocable_v<TCallable, Vulkan::Texture&>) {
            for (auto& [id, texture] : m_textures) {
                callable(texture);
            }
        } else if constexpr (std::is_invocable_v<TCallable, uint32_t, Vulkan::Texture&>) {
            for (auto& [id, texture] : m_textures) {
                callable(id, texture);
            }
        } else if constexpr (std::is_invocable_v<TCallable, Vulkan::Texture&, uint32_t>) {
            for (auto& [id, texture] : m_textures) {
                callable(texture, id);
            }
        }
    }

private:
    // GUIDs and Filenames
    std::unordered_map<uint32_t, std::string> m_assetPaths;
    std::unordered_map<std::string, uint32_t> m_assetIds;

    // Asset data
    std::unordered_map<uint32_t, Vulkan::Mesh> m_meshes;
    std::unordered_map<uint32_t, Vulkan::Texture> m_textures;

    static constexpr uint32_t k_nullIndex = std::numeric_limits<uint32_t>::max();
    static constexpr const char* k_modelDirPath = "assets/models/";
    static constexpr const char* k_textureDirPath = "assets/textures/";
};
} // namespace RDE
