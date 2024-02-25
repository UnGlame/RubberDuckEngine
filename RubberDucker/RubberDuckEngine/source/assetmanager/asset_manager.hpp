#pragma once
#include "vulkan/data_types/mesh.hpp"
#include "vulkan/data_types/texture.hpp"

#include <stbi/stb_image.h>

namespace RDE {
constexpr const char* k_modelDirPath = "assets/models/";
constexpr const char* k_textureDirPath = "assets/textures/";

class AssetManager
{
public:
    void loadModel(const char* modelPath);
    void loadModels(const char* folderPath = k_modelDirPath);
    void loadTexture(const char* texturePath);
    void loadTextures(const char* folderPath = k_textureDirPath);

    [[nodiscard]] __forceinline std::string getFileName(const char* filePath) const
    {
        const std::filesystem::path path(filePath);
        return path.filename().string();
    }

    [[nodiscard]] __forceinline std::string joinPath(const char* assetDir, const char* filename)
    {
        const auto fullPath = std::filesystem::path(assetDir) / std::filesystem::path(filename);
        return fullPath.string();
    }

    [[nodiscard]] __forceinline const char* getAssetPath(uint32_t id) const { return m_assetPaths.at(id).c_str(); }

    [[nodiscard]] __forceinline std::string getAssetName(uint32_t id) const
    {
        return getFileName(m_assetPaths.at(id).c_str());
    }

    [[nodiscard]] __forceinline uint32_t getAssetID(const char* assetPath) { return m_assetIDs.at(assetPath); };

    [[nodiscard]] __forceinline uint32_t getModelId(const char* modelName, const char* modelDir = k_modelDirPath)
    {
        const auto relativePath = joinPath(modelDir, modelName);
        return m_assetIDs.at(relativePath);
    };

    [[nodiscard]] __forceinline uint32_t getTextureId(const char* textureName,
                                                      const char* textureDir = k_textureDirPath)
    {
        const auto relativePath = std::filesystem::path(textureDir) / textureName;
        return m_assetIDs.at(relativePath.string());
    };

    [[nodiscard]] __forceinline uint32_t assetCount() { return static_cast<uint32_t>(m_assetIDs.size()); };

    [[nodiscard]] __forceinline Vulkan::Mesh& getMesh(uint32_t id) { return m_meshes[id]; };

    [[nodiscard]] __forceinline Vulkan::Mesh& getMesh(const char* assetName)
    {
        return m_meshes[m_assetIDs.at(assetName)];
    };

    [[nodiscard]] __forceinline uint32_t meshCount() { return static_cast<uint32_t>(m_meshes.size()); };

    [[nodiscard]] __forceinline Vulkan::Texture& getTexture(uint32_t id) { return m_textures[id]; };

    [[nodiscard]] __forceinline Vulkan::Texture& getTexture(const char* assetName)
    {
        return m_textures[m_assetIDs.at(assetName)];
    };

    [[nodiscard]] __forceinline uint32_t textureCount() { return static_cast<uint32_t>(m_textures.size()); };

    [[nodiscard]] __forceinline std::vector<std::string> getAssetNames() const
    {
        std::vector<std::string> assetNames{};
        assetNames.reserve(m_assetPaths.size());
        std::transform(m_assetPaths.begin(),
                       m_assetPaths.end(),
                       std::back_inserter(assetNames),
                       [this](const std::pair<uint32_t, std::string> pair) { return getAssetName(pair.first); });

        return assetNames;
    }

    [[nodiscard]] __forceinline std::vector<std::string> getAssetPaths() const
    {
        std::vector<std::string> assetPaths{};
        assetPaths.reserve(m_assetPaths.size());
        std::transform(m_assetPaths.begin(),
                       m_assetPaths.end(),
                       std::back_inserter(assetPaths),
                       [this](const std::pair<uint32_t, std::string> pair) { return getAssetPath(pair.first); });

        return assetPaths;
    }

    [[nodiscard]] __forceinline std::vector<std::string> getModelPaths() const
    {
        std::vector<std::string> modelPaths{};
        const auto assetPaths = getAssetPaths();

        std::copy_if(assetPaths.begin(),
                     assetPaths.end(),
                     std::back_inserter(modelPaths),
                     [](const std::string& assetPath) { return assetPath.find(k_modelDirPath) != std::string::npos; });

        return modelPaths;
    }

    [[nodiscard]] __forceinline std::vector<std::string> getTexturePaths() const
    {
        std::vector<std::string> texturePaths{};
        const auto assetPaths = getAssetPaths();

        std::copy_if(
            assetPaths.begin(), assetPaths.end(), std::back_inserter(texturePaths), [](const std::string& assetPath) {
                return assetPath.find(k_textureDirPath) != std::string::npos;
            });

        return texturePaths;
    }

    [[nodiscard]] __forceinline std::vector<std::string> getModelNames() const
    {
        const auto modelPaths = getModelPaths();
        std::vector<std::string> modelNames{};

        std::transform(modelPaths.begin(),
                       modelPaths.end(),
                       std::back_inserter(modelNames),
                       [this](const std::string& modelPath) { return getFileName(modelPath.c_str()); });

        return modelNames;
    }

    [[nodiscard]] __forceinline std::vector<std::string> getTextureNames() const
    {
        const auto texturePaths = getTexturePaths();
        std::vector<std::string> textureNames{};

        std::transform(texturePaths.begin(),
                       texturePaths.end(),
                       std::back_inserter(textureNames),
                       [this](const std::string& texturePath) { return getFileName(texturePath.c_str()); });

        return textureNames;
    }

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
    std::unordered_map<std::string, uint32_t> m_assetIDs;

    // Actual assets
    std::unordered_map<uint32_t, Vulkan::Mesh> m_meshes;
    std::unordered_map<uint32_t, Vulkan::Texture> m_textures;

    static constexpr uint32_t s_nullIndex = std::numeric_limits<uint32_t>::max();
};
} // namespace RDE
