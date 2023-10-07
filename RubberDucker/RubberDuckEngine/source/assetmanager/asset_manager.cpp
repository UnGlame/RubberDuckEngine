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
    if (m_assetIDs.find(modelPath) != m_assetIDs.end()) {
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
    uint32_t guid = static_cast<uint32_t>(m_assetIDs.size());
    m_assetIDs[modelPath] = guid;
    m_assetPaths[guid] = modelPath;

    m_meshes[guid] = std::move(mesh);
    RDELOG_INFO("Loaded model {0} with ID {1}", m_assetPaths[m_assetIDs[modelPath]], m_assetIDs[modelPath]);
}

void AssetManager::loadModels(const char* folderPath)
{
    std::filesystem::path path(folderPath);

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        RDELOG_ERROR("Folder path {0} invalid!", folderPath);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string filename = entry.path().filename().string();
        std::string filepath = folderPath + filename;

        loadModel(filepath.c_str());
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

    uint32_t guid = static_cast<uint32_t>(m_assetIDs.size());
    m_assetIDs[texturePath] = guid;
    m_assetPaths[guid] = texturePath;

    // Create vulkan texture and store
    m_textures[guid] = g_engine->renderer().createTextureResources(textureData);

    stbi_image_free(textureData.data);

    RDELOG_INFO("Loaded texture {0} with ID {1}", m_assetPaths[m_assetIDs[texturePath]], m_assetIDs[texturePath]);
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
} // namespace RDE
