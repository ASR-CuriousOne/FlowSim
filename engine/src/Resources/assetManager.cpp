#include <Ghost/Resources/assetManager.hpp>
#include <stdexcept>

#define TINYOBJLOADER_IMPLEMENTATION
#include <vendor/tiny_obj_loader.h>

namespace Ghost {
AssetManager::AssetManager(VulkanDevice &device) : m_device(device) {}

AssetManager::~AssetManager() { clear(); }

void AssetManager::clear() {
    m_models.clear();
    m_textureCache.clear();
}

std::shared_ptr<Mesh> AssetManager::getMesh(const std::string &filepath) {
    auto it = m_models.find(filepath);

    if (it != m_models.end()) {
        return it->second;
    }

    std::shared_ptr<Mesh> newModel = createModelFromFile(filepath);

    if (newModel) {
        m_models[filepath] = newModel;
    } else {
        throw std::runtime_error("Failed to load model: " + filepath);
    }

    return newModel;
}

std::shared_ptr<Mesh>
AssetManager::createModelFromFile(const std::filesystem::path &filepath,
                                  VertexType vertexType) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    if (vertexType == VertexType::Standard) {

        std::vector<StandardVertex> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<StandardVertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes) {
            for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
                StandardVertex v[3];

                for (int j = 0; j < 3; j++) {
                    auto idx = shape.mesh.indices[i + j];

                    v[j].position = {attrib.vertices[3 * idx.vertex_index + 0],
                                     attrib.vertices[3 * idx.vertex_index + 1],
                                     attrib.vertices[3 * idx.vertex_index + 2]};

                    if (!attrib.colors.empty()) {
                        v[j].color = {attrib.colors[3 * idx.vertex_index + 0],
                                      attrib.colors[3 * idx.vertex_index + 1],
                                      attrib.colors[3 * idx.vertex_index + 2]};
                    } else {
                        v[j].color = {1.0f, 1.0f, 1.0f};
                    }

                    if (idx.texcoord_index >= 0) {
                        v[j].uv = {
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            1.0f -
                                attrib.texcoords[2 * idx.texcoord_index + 1]};
                    }

                    v[j].normal = {attrib.normals[3 * idx.normal_index + 0],
                                   attrib.normals[3 * idx.normal_index + 1],
                                   attrib.normals[3 * idx.normal_index + 2]};

                    v[j].color = {1.f, 1.f, 1.f};
                }

                glm::vec3 edge1 = v[1].position - v[0].position;
                glm::vec3 edge2 = v[2].position - v[0].position;
                glm::vec2 deltaUV1 = v[1].uv - v[0].uv;
                glm::vec2 deltaUV2 = v[2].uv - v[0].uv;

                float f =
                    1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                glm::vec3 tangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent = glm::normalize(tangent);

                for (int j = 0; j < 3; j++) {
                    v[j].tangent = glm::vec4(tangent, 1.0f);

                    if (uniqueVertices.count(v[j]) == 0) {
                        uniqueVertices[v[j]] =
                            static_cast<uint32_t>(vertices.size());
                        vertices.push_back(v[j]);
                    }
                    indices.push_back(uniqueVertices[v[j]]);
                }
            }
        }
        return Mesh::create<StandardVertex>(m_device, std::span{vertices},
                                            std::span{indices}, vertexType);
    } else if (vertexType == VertexType::UI) {
        std::vector<UIVertex> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<UIVertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                UIVertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]};
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] =
                        static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        return Mesh::create<UIVertex>(m_device, std::span{vertices},
                                      std::span{indices}, vertexType);
    }

    throw std::runtime_error("Unsupported Vertex Type requested!");
}

} // namespace Ghost
