#include "precompiled/pch.hpp"

#include "assetmanager/asset_manager.hpp"

#include "core/main.hpp"
#include "vulkan/data_types/texture_data.hpp"
#include "vulkan/data_types/vertex.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace RDE {
const glm::vec3 k_defaultModelColor = {0.0f, 1.0f, 0.0f};

void AssetManager::loadModel(const char* modelPath)
{
    if (m_assetIds.find(modelPath) != m_assetIds.end()) {
        RDELOG_INFO("Model {0} already loaded!", modelPath);
        return;
    }

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    RDE_ASSERT_0(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath), warn + err);

    Vulkan::Mesh mesh{};
    std::unordered_map<Vulkan::Vertex, uint32_t> uniqueVertexIndices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vulkan::Vertex vertex{};

            const auto modelVertexX = attrib.vertices[3 * index.vertex_index + 0];
            const auto modelVertexY = attrib.vertices[3 * index.vertex_index + 1];
            const auto modelVertexZ = attrib.vertices[3 * index.vertex_index + 2];

            vertex.pos = {modelVertexX, modelVertexY, modelVertexZ};

            if (!attrib.texcoords.empty()) {
                const auto modelU = attrib.texcoords[2 * index.texcoord_index + 0];
                const auto modelV = attrib.texcoords[2 * index.texcoord_index + 1];
                vertex.texCoord = {modelU, 1.0f - modelV};
            }
            vertex.color = k_defaultModelColor;

            if (uniqueVertexIndices.count(vertex) == 0) {
                uniqueVertexIndices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }
            mesh.indices.push_back(uniqueVertexIndices[vertex]);
        }
    }
    uint32_t guid = static_cast<uint32_t>(m_assetIds.size());
    m_assetIds[modelPath] = guid;
    m_assetPaths[guid] = modelPath;

    m_meshes[guid] = std::move(mesh);
    RDELOG_INFO("Loaded model {0} with ID {1}", m_assetPaths[m_assetIds[modelPath]], m_assetIds[modelPath]);
}

void AssetManager::loadModels(const char* folderPath)
{
    std::filesystem::path path(folderPath);

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        RDELOG_ERROR("Folder path {0} invalid!", folderPath);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        const auto filename = entry.path().filename();

        if (filename.extension().string() != ".obj") {
            continue;
        }
        const auto filepath = folderPath / filename;
        loadModel(filepath.string().c_str());
    }

    std::ostringstream list;

    list << "Loaded models:\n";

    for (const auto& [id, mesh] : m_meshes) {
        list << "(" << id << ", " << &mesh << ") [" << m_assetPaths[id] << "]\n";
    }
    RDELOG_INFO(list.str().c_str());
}

void AssetManager::loadTexture(const char* texturePath)
{
    Vulkan::TextureData textureData;

    stbi_uc* pixels =
        stbi_load(texturePath, &textureData.texWidth, &textureData.texHeight, &textureData.texChannels, STBI_rgb_alpha);
    RDE_ASSERT_0(pixels, "Failed to load {}!", texturePath);

    textureData.data = pixels;

    uint32_t guid = static_cast<uint32_t>(m_assetIds.size());
    m_assetIds[texturePath] = guid;
    m_assetPaths[guid] = texturePath;

    // Create vulkan texture and store
    m_textures[guid] = g_engine->renderer().createTextureResources(textureData);

    stbi_image_free(textureData.data);

    RDELOG_INFO("Loaded texture {0} with ID {1}", m_assetPaths[m_assetIds[texturePath]], m_assetIds[texturePath]);
}

void AssetManager::loadTextures(const char* folderPath)
{
    std::filesystem::path path(folderPath);

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        RDELOG_ERROR("Folder path {0} invalid!", folderPath);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        const auto& fsPath = entry.path();
        std::string filename = fsPath.filename().string();
        std::string filepath = folderPath + filename;

        if (fsPath.extension() != ".png" && fsPath.extension() != ".jpg") {
            RDELOG_WARN("Texture {} is of {} extension, skipping", filename, fsPath.extension());
            continue;
        }
        loadTexture(filepath.c_str());
    }

    std::ostringstream list;

    list << "Loaded textures:\n";

    for (const auto& [id, texture] : m_textures) {
        list << "(" << id << ", " << &texture << ") [" << m_assetPaths[id] << "]\n";
    }
    RDELOG_INFO(list.str().c_str());
}

[[nodiscard]] std::string AssetManager::getFileName(const char* filePath) const
{
    const std::filesystem::path path(filePath);
    return path.filename().string();
}

[[nodiscard]] std::string AssetManager::joinPath(const char* assetDir, const char* filename) const
{
    const auto fullPath = std::filesystem::path(assetDir) / std::filesystem::path(filename);
    return fullPath.string();
}

[[nodiscard]] const char* AssetManager::getAssetPath(uint32_t id) const
{
    return m_assetPaths.at(id).c_str();
}

[[nodiscard]] std::string AssetManager::getAssetName(uint32_t id) const
{
    return getFileName(m_assetPaths.at(id).c_str());
}

[[nodiscard]] uint32_t AssetManager::getAssetId(const char* assetPath) const
{
    return m_assetIds.at(assetPath);
}

[[nodiscard]] uint32_t AssetManager::getModelId(const char* modelName, const char* modelDir) const
{
    const auto relativePath = joinPath(modelDir, modelName);
    return m_assetIds.at(relativePath);
}

[[nodiscard]] uint32_t AssetManager::getTextureId(const char* textureName, const char* textureDir) const
{
    const auto relativePath = std::filesystem::path(textureDir) / textureName;
    return m_assetIds.at(relativePath.string());
}

[[nodiscard]] uint32_t AssetManager::assetCount() const
{
    return static_cast<uint32_t>(m_assetIds.size());
}

[[nodiscard]] Vulkan::Mesh& AssetManager::getMesh(uint32_t id)
{
    return m_meshes[id];
}

[[nodiscard]] Vulkan::Mesh& AssetManager::getMesh(const char* assetName)
{
    return m_meshes[m_assetIds.at(assetName)];
}

[[nodiscard]] uint32_t AssetManager::meshCount() const
{
    return static_cast<uint32_t>(m_meshes.size());
}

[[nodiscard]] Vulkan::Texture& AssetManager::getTexture(uint32_t id)
{
    return m_textures[id];
}

[[nodiscard]] Vulkan::Texture& AssetManager::getTexture(const char* assetName)
{
    return m_textures[m_assetIds.at(assetName)];
}

[[nodiscard]] uint32_t AssetManager::textureCount() const
{
    return static_cast<uint32_t>(m_textures.size());
}

[[nodiscard]] std::vector<std::string> AssetManager::getAssetNames() const
{
    std::vector<std::string> assetNames{};
    assetNames.reserve(m_assetPaths.size());
    std::transform(m_assetPaths.begin(),
                   m_assetPaths.end(),
                   std::back_inserter(assetNames),
                   [this](const std::pair<uint32_t, std::string> pair) { return getAssetName(pair.first); });

    return assetNames;
}

[[nodiscard]] std::vector<std::string> AssetManager::getAssetPaths() const
{
    std::vector<std::string> assetPaths{};
    assetPaths.reserve(m_assetPaths.size());
    std::transform(m_assetPaths.begin(),
                   m_assetPaths.end(),
                   std::back_inserter(assetPaths),
                   [this](const std::pair<uint32_t, std::string> pair) { return getAssetPath(pair.first); });

    return assetPaths;
}

[[nodiscard]] std::vector<std::string> AssetManager::getModelPaths() const
{
    std::vector<std::string> modelPaths{};
    const auto assetPaths = getAssetPaths();

    std::copy_if(assetPaths.begin(),
                 assetPaths.end(),
                 std::back_inserter(modelPaths),
                 [](const std::string& assetPath) { return assetPath.find(k_modelDirPath) != std::string::npos; });

    return modelPaths;
}

[[nodiscard]] std::vector<std::string> AssetManager::getTexturePaths() const
{
    std::vector<std::string> texturePaths{};
    const auto assetPaths = getAssetPaths();

    std::copy_if(assetPaths.begin(),
                 assetPaths.end(),
                 std::back_inserter(texturePaths),
                 [](const std::string& assetPath) { return assetPath.find(k_textureDirPath) != std::string::npos; });

    return texturePaths;
}

[[nodiscard]] std::vector<std::string> AssetManager::getModelNames() const
{
    const auto modelPaths = getModelPaths();
    std::vector<std::string> modelNames{};

    std::transform(modelPaths.begin(),
                   modelPaths.end(),
                   std::back_inserter(modelNames),
                   [this](const std::string& modelPath) { return getFileName(modelPath.c_str()); });

    return modelNames;
}

[[nodiscard]] std::vector<std::string> AssetManager::getTextureNames() const
{
    const auto texturePaths = getTexturePaths();
    std::vector<std::string> textureNames{};

    std::transform(texturePaths.begin(),
                   texturePaths.end(),
                   std::back_inserter(textureNames),
                   [this](const std::string& texturePath) { return getFileName(texturePath.c_str()); });

    return textureNames;
}

} // namespace RDE
