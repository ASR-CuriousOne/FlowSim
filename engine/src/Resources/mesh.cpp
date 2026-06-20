#include <Ghost/Resources/mesh.hpp>

namespace Ghost {
Mesh::Mesh(VulkanDevice &device, std::span<const std::byte> vertexData,
           uint32_t vertexCount, const std::span<const uint32_t> &indicies,
           VertexType vertexType)
    : m_device(device), m_vertexCount(static_cast<uint32_t>(vertexCount)),
      m_indexCount(static_cast<uint32_t>(indicies.size())),
      m_vertexType(vertexType) {

    vk::DeviceSize bufferSize = vertexData.size_bytes();

    GhostBuffer stagingVertexBuffer(
        m_device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    stagingVertexBuffer.writeToBuffer(vertexData);

    m_vertexBuffer = std::make_unique<GhostBuffer>(
        m_device, bufferSize,
        vk::BufferUsageFlagBits::eTransferDst |
            vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    auto commandBuffer = m_device.beginSingleTimeCommands();

    vk::BufferCopy copyRegion;
    copyRegion.setSrcOffset(0).setDstOffset(0).setSize(bufferSize);
    commandBuffer.copyBuffer(stagingVertexBuffer.getBuffer(),
                             m_vertexBuffer->getBuffer(), copyRegion);

    m_device.endSingleTimeCommands(commandBuffer);

    vk::DeviceSize indexBufferSize = sizeof(uint32_t) * m_indexCount;
    GhostBuffer stagingIndexBuffer(
        m_device, indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    stagingIndexBuffer.writeToBuffer(std::span<const uint32_t>(indicies));

    m_indexBuffer =
        std::make_unique<GhostBuffer>(m_device, indexBufferSize,
                                      vk::BufferUsageFlagBits::eTransferDst |
                                          vk::BufferUsageFlagBits::eIndexBuffer,
                                      vk::MemoryPropertyFlagBits::eDeviceLocal);

    commandBuffer = m_device.beginSingleTimeCommands();

    vk::BufferCopy indexCopyRegion;
    indexCopyRegion.setSrcOffset(0).setDstOffset(0).setSize(indexBufferSize);
    commandBuffer.copyBuffer(stagingIndexBuffer.getBuffer(),
                             m_indexBuffer->getBuffer(), indexCopyRegion);

    m_device.endSingleTimeCommands(commandBuffer);
}

Mesh::~Mesh() {}

void Mesh::bind(const vk::CommandBuffer &commandBuffer) {
    vk::Buffer vertexBuffers[] = {m_vertexBuffer->getBuffer()};
    vk::Buffer indexBuffer = m_indexBuffer->getBuffer();
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
}

void Mesh::draw(const vk::CommandBuffer &commandBuffer) {
    commandBuffer.drawIndexed(static_cast<uint32_t>(m_indexCount), 1, 0, 0, 0);
}

} // namespace Ghost
