#include "precompiled/pch.hpp"

#include "assetmanager/asset_manager.hpp"
#include "vulkan/vertex.hpp"

#define	STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace RDE {

	void AssetManager::loadModel(const char* modelPath)
	{
		if (m_assetIDs.find(modelPath) != m_assetIDs.end()) {
			RDE_LOG_INFO("Model {0} already loaded!", modelPath);
			return;
		}

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		RDE_ASSERT_0(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath), warn + err);

		Vulkan::Mesh mesh;
		std::unordered_map<Vulkan::Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vulkan::Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				if (!attrib.texcoords.empty()) {
					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
					mesh.vertices.push_back(vertex);
				}
				mesh.indices.push_back(uniqueVertices[vertex]);
			}
		}
		uint32_t guid = static_cast<uint32_t>(m_assetIDs.size());
		m_assetIDs[modelPath] = guid;
		m_assetNames[guid] = modelPath;

		m_meshes[guid] = std::move(mesh);
		RDE_LOG_INFO("Loaded model {0} with ID {1}", m_assetNames[m_assetIDs[modelPath]], m_assetIDs[modelPath]);
	}

	void AssetManager::loadModels(const char* folderPath)
	{
		std::filesystem::path path(folderPath);
		std::unordered_map<uint32_t, Vulkan::Mesh*> meshes;

		if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
			RDE_LOG_ERROR("Folder path {0} invalid!", folderPath);
			return;
		}
	
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			std::string filename = entry.path().filename().string();
			std::string filepath = folderPath + filename;

			loadModel(filepath.c_str());
		}

		std::ostringstream list;

		for (const auto& [id, mesh] : m_meshes) {
			list << "(" << id << ", " << &mesh << " [" << m_assetNames[id] << "]) ";
		}
		RDE_LOG_INFO(list.str().c_str());
	}

	stbi_uc* AssetManager::loadTexture(const char* texturePath, int& texWidth, int& texHeight, int& texChannels)
	{

		stbi_uc* pixels = stbi_load(texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		RDE_ASSERT_0(pixels, "Failed to load {}!", texturePath);

		return pixels;
	}

	void AssetManager::freeTexture(stbi_uc* loadedTexture)
	{
		stbi_image_free(loadedTexture);
	}
}