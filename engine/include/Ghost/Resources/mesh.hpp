#pragma once
#include <Ghost/Core/ghostBuffer.hpp>
#include <Ghost/Resources/vertex.hpp>

namespace Ghost {
class Mesh {
  public:
    Mesh(VulkanDevice &device, std::span<const std::byte> vertexData,
         uint32_t vertexCount, const std::span<const uint32_t> &indicies,
         VertexType vertexType);
    ~Mesh();

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    template <typename T>
    static std::shared_ptr<Mesh>
    create(VulkanDevice &device, std::span<const T> vertices,
           std::span<const uint32_t> indices, VertexType vertexType) {
        std::span<const std::byte> byteSpan = std::as_bytes(vertices);

        return std::make_shared<Mesh>(device, byteSpan, vertices.size(),
                                      indices, vertexType);
    }

    void bind(const vk::CommandBuffer &commandBuffer);
    void draw(const vk::CommandBuffer &commandBuffer);

    VertexType getVertexType() const { return m_vertexType; }

  private:
    VulkanDevice &m_device;
    std::unique_ptr<GhostBuffer> m_vertexBuffer;
    std::unique_ptr<GhostBuffer> m_indexBuffer;
    uint32_t m_vertexCount;
    uint32_t m_indexCount;
    VertexType m_vertexType;
};
} // namespace Ghost
