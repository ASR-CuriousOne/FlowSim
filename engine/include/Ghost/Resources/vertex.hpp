#pragma once
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.hpp>

namespace Ghost {
enum class VertexType {
    Standard,
    UI,
};

struct StandardVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec4 tangent;

    static std::vector<vk::VertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const StandardVertex &other) const {
        return position == other.position && color == other.color &&
               uv == other.uv && normal == other.normal &&
               tangent == other.tangent;
    }
};

struct UIVertex {
    glm::vec3 position;
    glm::vec2 uv;

    static std::vector<vk::VertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const UIVertex &other) const {
        return position == other.position && uv == other.uv;
    }
};

template <typename T, typename... Rest>
void hashCombine(std::size_t &seed, const T &v, const Rest &...rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}
} // namespace Ghost

namespace std {
template <> struct hash<Ghost::StandardVertex> {
    size_t operator()(const Ghost::StandardVertex &vertex) const {
        size_t seed = 0;
        Ghost::hashCombine(seed, vertex.position, vertex.color, vertex.normal,
                           vertex.uv, vertex.tangent);
        return seed;
    }
};

template <> struct hash<Ghost::UIVertex> {
    size_t operator()(const Ghost::UIVertex &vertex) const {
        size_t seed = 0;
        Ghost::hashCombine(seed, vertex.position, vertex.uv);
        return seed;
    }
};

} // namespace std
