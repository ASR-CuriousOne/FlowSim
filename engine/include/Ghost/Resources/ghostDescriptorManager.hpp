#pragma once
#include <Ghost/Resources/ghostDescriptors.hpp>
#include <Ghost/Core/vulkanDevice.hpp>
#include <unordered_map>

namespace Ghost {
class GhostDescriptorManager {
  public:
    GhostDescriptorManager(VulkanDevice &device);
    ~GhostDescriptorManager() = default;

    GhostDescriptorManager(const GhostDescriptorManager &) = delete;
    GhostDescriptorManager &operator=(const GhostDescriptorManager &) = delete;

    void registerLayout(const std::string &name,
                        std::unique_ptr<GhostDescriptorSetLayout> layout);

    GhostDescriptorSetLayout &getLayout(const std::string &name);

    std::vector<vk::raii::DescriptorSet>
    allocateSets(const std::string &layoutName, uint32_t count = 1);

  private:
    VulkanDevice &m_device;
    std::unordered_map<std::string, std::unique_ptr<GhostDescriptorSetLayout>>
        m_layouts;
    std::unique_ptr<GhostDescriptorPool> m_globalPool;
};
} // namespace Ghost
