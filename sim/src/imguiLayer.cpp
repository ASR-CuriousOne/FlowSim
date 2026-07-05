#include <Ghost/Core/vulkanDevice.hpp>
#include <Ghost/Systems/ghostRenderer.hpp>
#include <Sim/imguiLayer.hpp>
#include <Sim/windowGLFW.hpp>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

ImGuiLayer::ImGuiLayer(Ghost::VulkanDevice &device, WindowGLFW &window,
                       vk::RenderPass renderPass, vk::Instance instance)
    : m_device(device) {
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000}};

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    m_descriptorPool = vk::raii::DescriptorPool(m_device, poolInfo);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo = {};

    initInfo.Instance = static_cast<VkInstance>(instance);
    initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(
        m_device.operator const vk::PhysicalDevice &());
    initInfo.Device = m_device.getRawDeviceHandle();

    initInfo.QueueFamily =
        m_device.getQueueFamilyIndices().graphicsFamily.value();
    initInfo.Queue = static_cast<VkQueue>(m_device.getGraphicsQueue());
    initInfo.DescriptorPool = static_cast<VkDescriptorPool>(*m_descriptorPool);
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = Ghost::MAX_FRAMES_IN_FLIGHT;
    initInfo.PipelineInfoMain.RenderPass = renderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
}

ImGuiLayer::~ImGuiLayer() {
    m_device->waitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::beginFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render(const vk::raii::CommandBuffer &commandBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(), static_cast<VkCommandBuffer>(*commandBuffer));
}
