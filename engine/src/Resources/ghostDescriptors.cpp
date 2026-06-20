#include <Ghost/Resources/ghostDescriptors.hpp>
#include <stdexcept>
#include <vector>

namespace Ghost {
GhostDescriptorSetLayout::Builder &
GhostDescriptorSetLayout::Builder::addBinding(uint32_t binding,
                                              vk::DescriptorType descriptorType,
                                              vk::ShaderStageFlags stageFlags,
                                              uint32_t count) {

    if (m_bindings.count(binding) != 0) {
        throw std::runtime_error("Binding already in use");
    }

    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.setBinding(binding)
        .setDescriptorType(descriptorType)
        .setDescriptorCount(count)
        .setStageFlags(stageFlags);

    m_bindings[binding] = layoutBinding;

    return *this;
}

std::unique_ptr<GhostDescriptorSetLayout>
GhostDescriptorSetLayout::Builder::build() const {
    return std::make_unique<GhostDescriptorSetLayout>(m_device, m_bindings);
}

GhostDescriptorSetLayout::GhostDescriptorSetLayout(
    VulkanDevice &device,
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : m_device{device}, m_bindings{bindings} {

    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto &[key, value] : bindings) {
        setLayoutBindings.push_back(value);
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo
        .setBindingCount(static_cast<uint32_t>(setLayoutBindings.size()))
        .setPBindings(setLayoutBindings.data());

    m_descriptorSetLayout =
        vk::raii::DescriptorSetLayout(m_device, descriptorSetLayoutInfo);
}

GhostDescriptorSetLayout::~GhostDescriptorSetLayout() {}

GhostDescriptorPool::Builder &
GhostDescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType,
                                          uint32_t count) {
    m_poolSizes.push_back({descriptorType, count});
    return *this;
}

GhostDescriptorPool::Builder &GhostDescriptorPool::Builder::setPoolFlags(
    vk::DescriptorPoolCreateFlags flags) {
    m_poolFlags = flags;
    return *this;
}

GhostDescriptorPool::Builder &
GhostDescriptorPool::Builder::setMaxSets(uint32_t count) {
    m_maxSets = count;
    return *this;
}

std::unique_ptr<GhostDescriptorPool>
GhostDescriptorPool::Builder::build() const {
    return std::make_unique<GhostDescriptorPool>(m_device, m_maxSets,
                                                 m_poolFlags, m_poolSizes);
}

GhostDescriptorPool::GhostDescriptorPool(
    VulkanDevice &device, uint32_t maxSets,
    vk::DescriptorPoolCreateFlags poolFlags,
    const std::vector<vk::DescriptorPoolSize> &poolSizes)
    : m_device{device} {

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
        .setPPoolSizes(poolSizes.data())
        .setMaxSets(maxSets)
        .setFlags(poolFlags);

    m_descriptorPool = vk::raii::DescriptorPool(m_device, descriptorPoolInfo);
}

GhostDescriptorPool::~GhostDescriptorPool() {}

std::vector<vk::raii::DescriptorSet>
GhostDescriptorPool::allocateDescriptorSets(
    uint32_t descriptorSetCount,
    const vk::DescriptorSetLayout &descriptorSetLayout) {

    std::vector<vk::DescriptorSetLayout> layouts(descriptorSetCount,
                                                 descriptorSetLayout);

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.setDescriptorPool(*m_descriptorPool)
        .setDescriptorSetCount(descriptorSetCount)
        .setPSetLayouts(layouts.data());

    return m_device->allocateDescriptorSets(allocInfo);
}

GhostDescriptorWriter::GhostDescriptorWriter(
    GhostDescriptorSetLayout &setLayout)
    : m_setLayout{setLayout} {}

GhostDescriptorWriter &
GhostDescriptorWriter::writeBuffer(uint32_t binding,
                                   const vk::DescriptorBufferInfo *bufferInfo) {

    const auto &bindings = m_setLayout.getBindings();
    if (bindings.count(binding) == 0) {
        throw std::runtime_error("Layout does not contain specified binding");
    }

    vk::WriteDescriptorSet write{};
    write.setDescriptorType(bindings.at(binding).descriptorType)
        .setDstBinding(binding)
        .setDescriptorCount(1)
        .setPBufferInfo(bufferInfo);

    m_writes.push_back(write);
    return *this;
}

GhostDescriptorWriter &
GhostDescriptorWriter::writeImage(uint32_t binding,
                                  const vk::DescriptorImageInfo *imageInfo) {

    const auto &bindings = m_setLayout.getBindings();
    if (bindings.count(binding) == 0) {
        throw std::runtime_error("Layout does not contain specified binding");
    }

    vk::WriteDescriptorSet write{};
    write.setDescriptorType(bindings.at(binding).descriptorType)
        .setDstBinding(binding)
        .setDescriptorCount(1)
        .setPImageInfo(imageInfo);

    m_writes.push_back(write);
    return *this;
}

void GhostDescriptorWriter::build(const vk::raii::DescriptorSet &set,
                                  const VulkanDevice &device) {
    for (auto &write : m_writes) {
        write.setDstSet(*set);
    }

    device->updateDescriptorSets(m_writes, nullptr);
}

} // namespace Ghost
