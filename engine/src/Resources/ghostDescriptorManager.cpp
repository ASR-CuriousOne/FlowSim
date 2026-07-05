#include <Ghost/Resources/ghostDescriptorManager.hpp>

namespace Ghost {
GhostDescriptorManager::GhostDescriptorManager(VulkanDevice &device)
    : m_device(device) {
    m_globalPool =
        GhostDescriptorPool::Builder(m_device)
            .setMaxSets(2000)
            .addPoolSize(vk::DescriptorType::eUniformBuffer, 1000)
            .addPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000)
			.addPoolSize(vk::DescriptorType::eStorageImage, 1000)
            .setPoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .build();
}

void GhostDescriptorManager::registerLayout(
    const std::string &name, std::unique_ptr<GhostDescriptorSetLayout> layout) {
    if (m_layouts.contains(name)) {
        throw std::runtime_error("Layout with name '" + name +
                                 "' already exists!");
    }
    m_layouts[name] = std::move(layout);
}

GhostDescriptorSetLayout &
GhostDescriptorManager::getLayout(const std::string &name) {
    if (!m_layouts.contains(name)) {
        throw std::runtime_error("Layout with name '" + name + "' not found!");
    }
    return *m_layouts[name];
}

std::vector<vk::raii::DescriptorSet>
GhostDescriptorManager::allocateSets(const std::string &layoutName,
                                     uint32_t count) {
    auto &layout = getLayout(layoutName);
    return m_globalPool->allocateDescriptorSets(
        count, layout.getDescriptorSetLayout());
}

} // namespace Ghost
