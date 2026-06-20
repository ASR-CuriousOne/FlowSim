#include <Ghost/Resources/vertex.hpp>

namespace Ghost {
std::vector<vk::VertexInputBindingDescription>
StandardVertex::getBindingDescriptions() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0]
        .setBinding(0)
        .setStride(sizeof(StandardVertex))
        .setInputRate(vk::VertexInputRate::eVertex);
    return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription>
StandardVertex::getAttributeDescriptions() {
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(5);
    attributeDescriptions[0]
        .setBinding(0)
        .setLocation(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setOffset(offsetof(StandardVertex, position));
    attributeDescriptions[1]
        .setBinding(0)
        .setLocation(1)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setOffset(offsetof(StandardVertex, color));
    attributeDescriptions[2]
        .setBinding(0)
        .setLocation(2)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setOffset(offsetof(StandardVertex, uv));
    attributeDescriptions[3]
        .setBinding(0)
        .setLocation(3)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setOffset(offsetof(StandardVertex, normal));
    attributeDescriptions[4]
        .setBinding(0)
        .setLocation(4)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setOffset(offsetof(StandardVertex, tangent));

    return attributeDescriptions;
}

std::vector<vk::VertexInputBindingDescription>
UIVertex::getBindingDescriptions() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0]
        .setBinding(0)
        .setStride(sizeof(UIVertex))
        .setInputRate(vk::VertexInputRate::eVertex);
    return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription>
UIVertex::getAttributeDescriptions() {
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0]
        .setBinding(0)
        .setLocation(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setOffset(offsetof(UIVertex, position));
    attributeDescriptions[1]
        .setBinding(0)
        .setLocation(2)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setOffset(offsetof(UIVertex, uv));
    return attributeDescriptions;
}

} // namespace Ghost
