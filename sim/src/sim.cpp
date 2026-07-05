#include "imgui.h"
#include <Ghost/Resources/vertex.hpp>
#include <Ghost/Utils/utils.hpp>
#include <Sim/pushConstants.hpp>
#include <Sim/sim.hpp>
#include <atomic>
#include <chrono>

namespace FluidSim {

extern std::atomic<bool> g_shutdownRequested;

Sim::Sim() : m_window() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

    Ghost::ForwardRendererConfig rendererConfig;
    rendererConfig.instanceExtensions = extensions;
    rendererConfig.windowHeight = m_window.HEIGHT;
    rendererConfig.windowWidth = m_window.WIDTH;

    rendererConfig.surfaceCreateCallback = [this](VkInstance instance) {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
        return surface;
    };

    m_forwardRenderer =
        std::make_unique<Ghost::ForwardRenderer>(rendererConfig);

    m_descriptorManager = std::make_unique<Ghost::GhostDescriptorManager>(
        m_forwardRenderer->getDevice());
}

void Sim::run() {
    std::clog << "Initializing Sim..." << std::endl;

    onInit();

    std::clog << "Sim Loop: Started..." << std::endl;

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (m_isRunning && !m_window.shouldClose()) {
        glfwPollEvents();

        if (m_window.m_framebufferResized) {
            int width = 0, height = 0;
            glfwGetFramebufferSize(m_window, &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(m_window, &width, &height);
                glfwWaitEvents();
            }
            m_forwardRenderer->resize(width, height);
            m_window.m_framebufferResized = false;
        }

        auto newTime = std::chrono::high_resolution_clock::now();
        float deltaTime =
            std::chrono::duration<float, std::chrono::seconds::period>(
                newTime - currentTime)
                .count();
        currentTime = newTime;

        onUpdate(deltaTime);

        if (auto &commandBuffer = m_forwardRenderer->beginFrame();
            m_forwardRenderer->isFrameInProgress()) {
            Ghost::FrameInfo frameInfo{
                static_cast<uint32_t>(m_forwardRenderer->getFrameIndex()),
                deltaTime, commandBuffer};

            dispatchCompute(commandBuffer, deltaTime);

            m_forwardRenderer->beginSwapChainRenderPass(commandBuffer);

            onRender(frameInfo);

            m_forwardRenderer->endSwapChainRenderPass(commandBuffer);
            m_forwardRenderer->endFrame();
        }

        if (g_shutdownRequested) {
            std::clog << " Shutdown Requested" << std::endl;

            close();
        }
    }

    m_forwardRenderer->getDevice()->waitIdle();

    std::clog << "Sim Loop: Stopped." << std::endl;

    std::clog << "Shutting down Sim...." << std::endl;
    onShutdown();
    std::clog << "Sim closed...." << std::endl;
}

void Sim::onInit() {
    m_imguiLayer =
        std::make_unique<ImGuiLayer>(m_forwardRenderer->getDevice(), m_window,
                                     m_forwardRenderer->getRenderPass(),
                                     *(m_forwardRenderer->getInstance()));
    generateQuad();
    createComputePipeline();
    createComputeResources();
    createPipeline();
    createMaterial();
}

void Sim::createPipeline() {
    m_descriptorManager->registerLayout(
        "FluidMaterialLayout",
        Ghost::GhostDescriptorSetLayout::Builder(m_forwardRenderer->getDevice())
            .addBinding(0, vk::DescriptorType::eCombinedImageSampler,
                        vk::ShaderStageFlagBits::eFragment)
            .addBinding(1, vk::DescriptorType::eCombinedImageSampler,
                        vk::ShaderStageFlagBits::eFragment)
            .build());

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
        m_descriptorManager->getLayout("FluidMaterialLayout")
            .getDescriptorSetLayout()};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
    m_fluidGraphicsPipelineLayout = vk::raii::PipelineLayout(
        m_forwardRenderer->getDevice(), pipelineLayoutInfo);

    Ghost::PipelineConfigInfo configInfo;

    Ghost::PipelineConfigInfo::defaultConfig(configInfo);

    configInfo.renderPass = m_forwardRenderer->getRenderPass();
    configInfo.pipelineLayout = *m_fluidGraphicsPipelineLayout;

    auto bindingDescriptions = Ghost::StandardVertex::getBindingDescriptions();
    auto attributeDescriptions =
        Ghost::StandardVertex::getAttributeDescriptions();

    configInfo.vertexInputInfo.setVertexBindingDescriptions(
        bindingDescriptions);
    configInfo.vertexInputInfo.setVertexAttributeDescriptions(
        attributeDescriptions);

    configInfo.rasterizationInfo.setCullMode(vk::CullModeFlagBits::eNone);

    auto vertCode = Ghost::Utils::readFile("sim/shaders/fluid.vert.spv");
    auto fragCode = Ghost::Utils::readFile("sim/shaders/fluid.frag.spv");

    m_fluidGraphicsPipeline = std::make_shared<Ghost::GhostGraphicsPipeline>(
        m_forwardRenderer->getDevice(), vertCode, fragCode, configInfo);
}

void Sim::generateQuad() {
    std::vector<Ghost::StandardVertex> quadVertexData = {
        {{-1.0f, 1.0f, 0.0f}, {0, 0, 0}, {0.0f, 1.0f}, {0, 0, 0}, {0, 0, 0, 0}},
        {{1.0f, 1.0f, 0.0f}, {0, 0, 0}, {1.0f, 1.0f}, {0, 0, 0}, {0, 0, 0, 0}},
        {{1.0f, -1.0f, 0.0f}, {0, 0, 0}, {1.0f, 0.0f}, {0, 0, 0}, {0, 0, 0, 0}},
        {{-1.0f, -1.0f, 0.0f},
         {0, 0, 0},
         {0.0f, 0.0f},
         {0, 0, 0},
         {0, 0, 0, 0}}};

    std::vector<uint32_t> indicies = {0, 1, 2, 2, 3, 0};

    m_quad = Ghost::Mesh::create<Ghost::StandardVertex>(
        m_forwardRenderer->getDevice(), std::span{quadVertexData},
        std::span{indicies}, Ghost::VertexType::Standard);
}

void Sim::createMaterial() {
    m_fluidMaterial = std::make_shared<FluidMaterial>(
        m_forwardRenderer->getDevice(), *m_descriptorManager,
        m_fluidGraphicsPipeline, 2);

    m_fluidMaterial->setTextures(m_velocityTex[m_readIndex],
                                 m_densityTex[m_readIndex]);
}

void Sim::createComputeResources() {
    auto &device = m_forwardRenderer->getDevice();
    uint32_t width = m_window.WIDTH;
    uint32_t height = m_window.HEIGHT;

    vk::Format format = vk::Format::eR32G32B32A32Sfloat;

    for (int i = 0; i < 2; i++) {
        m_velocityTex[i] = std::make_shared<Ghost::StorageTexture>(
            device, width, height, format);
        m_densityTex[i] = std::make_shared<Ghost::StorageTexture>(
            device, width, height, format);
        m_pressureTex[i] = std::make_shared<Ghost::StorageTexture>(
            device, width, height, format);
    }
    m_divergenceTex =
        std::make_shared<Ghost::StorageTexture>(device, width, height, format);

    auto cmd = m_forwardRenderer->getDevice().beginSingleTimeCommands();

    for (int i = 0; i < 2; i++) {
        m_velocityTex[i]->transitionLayout(cmd, vk::ImageLayout::eGeneral);
        m_densityTex[i]->transitionLayout(cmd, vk::ImageLayout::eGeneral);
        m_pressureTex[i]->transitionLayout(cmd, vk::ImageLayout::eGeneral);
    }
    m_divergenceTex->transitionLayout(cmd, vk::ImageLayout::eGeneral);

    m_forwardRenderer->getDevice().endSingleTimeCommands(cmd);
}

void Sim::createComputePipeline() {
    m_descriptorManager->registerLayout(
        "FluidComputeLayout",
        Ghost::GhostDescriptorSetLayout::Builder(m_forwardRenderer->getDevice())
            .addBinding(0, vk::DescriptorType::eStorageImage,
                        vk::ShaderStageFlagBits::eCompute)
            .addBinding(1, vk::DescriptorType::eStorageImage,
                        vk::ShaderStageFlagBits::eCompute)
            .addBinding(2, vk::DescriptorType::eStorageImage,
                        vk::ShaderStageFlagBits::eCompute)
            .addBinding(3, vk::DescriptorType::eStorageImage,
                        vk::ShaderStageFlagBits::eCompute)
            .build());

    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimComputePushConstant);

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
        m_descriptorManager->getLayout("FluidComputeLayout")
            .getDescriptorSetLayout()};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
    pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);

    m_computePipelineLayout = vk::raii::PipelineLayout(
        m_forwardRenderer->getDevice(), pipelineLayoutInfo);

    auto advectCode = Ghost::Utils::readFile("sim/shaders/advect.comp.spv");
    m_advectPipeline = std::make_shared<Ghost::GhostComputePipeline>(
        m_forwardRenderer->getDevice(), advectCode, *m_computePipelineLayout);

    auto divCode = Ghost::Utils::readFile("sim/shaders/divergence.comp.spv");
    m_divergencePipeline = std::make_shared<Ghost::GhostComputePipeline>(
        m_forwardRenderer->getDevice(), divCode, *m_computePipelineLayout);

    auto jacobiCode = Ghost::Utils::readFile("sim/shaders/jacobi.comp.spv");
    m_jacobiPipeline = std::make_shared<Ghost::GhostComputePipeline>(
        m_forwardRenderer->getDevice(), jacobiCode, *m_computePipelineLayout);

    auto projectCode = Ghost::Utils::readFile("sim/shaders/project.comp.spv");
    m_projectPipeline = std::make_shared<Ghost::GhostComputePipeline>(
        m_forwardRenderer->getDevice(), projectCode, *m_computePipelineLayout);

    auto splatCode = Ghost::Utils::readFile("sim/shaders/splat.comp.spv");
    m_splatPipeline = std::make_shared<Ghost::GhostComputePipeline>(
        m_forwardRenderer->getDevice(), splatCode, *m_computePipelineLayout);

    m_computeDescriptorSets =
        m_descriptorManager->allocateSets("FluidComputeLayout", 8);
}

void Sim::updateComputeDescriptors() {
    auto &layout = m_descriptorManager->getLayout("FluidComputeLayout");

    vk::DescriptorImageInfo velRead =
        m_velocityTex[m_readIndex]->descriptorInfo();
    vk::DescriptorImageInfo velWrite =
        m_velocityTex[m_writeIndex]->descriptorInfo();
    vk::DescriptorImageInfo denRead =
        m_densityTex[m_readIndex]->descriptorInfo();
    vk::DescriptorImageInfo denWrite =
        m_densityTex[m_writeIndex]->descriptorInfo();
    vk::DescriptorImageInfo div = m_divergenceTex->descriptorInfo();
    vk::DescriptorImageInfo p0 = m_pressureTex[0]->descriptorInfo();
    vk::DescriptorImageInfo p1 = m_pressureTex[1]->descriptorInfo();

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &velRead)
        .writeImage(1, &velRead)
        .writeImage(2, &velWrite)
        .writeImage(3, &velWrite)
        .build(m_computeDescriptorSets[0], m_forwardRenderer->getDevice());

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &velWrite)
        .writeImage(1, &denRead)
        .writeImage(2, &denWrite)
        .writeImage(3, &denWrite)
        .build(m_computeDescriptorSets[1], m_forwardRenderer->getDevice());

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &velWrite)
        .writeImage(1, &div)
        .writeImage(2, &div)
        .writeImage(3, &div)
        .build(m_computeDescriptorSets[2], m_forwardRenderer->getDevice());

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &p0)
        .writeImage(1, &div)
        .writeImage(2, &p1)
        .writeImage(3, &p1)
        .build(m_computeDescriptorSets[3], m_forwardRenderer->getDevice());

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &p1)
        .writeImage(1, &div)
        .writeImage(2, &p0)
        .writeImage(3, &p0)
        .build(m_computeDescriptorSets[4], m_forwardRenderer->getDevice());

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &velWrite)
        .writeImage(1, &p0)
        .writeImage(2, &velWrite)
        .writeImage(3, &velWrite)
        .build(m_computeDescriptorSets[5], m_forwardRenderer->getDevice());

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &velRead)
        .writeImage(1, &velRead)
        .writeImage(2, &velRead)
        .writeImage(3, &velRead)
        .build(m_computeDescriptorSets[6], m_forwardRenderer->getDevice());

    Ghost::GhostDescriptorWriter(layout)
        .writeImage(0, &denRead)
        .writeImage(1, &denRead)
        .writeImage(2, &denRead)
        .writeImage(3, &denRead)
        .build(m_computeDescriptorSets[7], m_forwardRenderer->getDevice());

    m_fluidMaterial->setTextures(m_velocityTex[m_readIndex],
                                 m_densityTex[m_readIndex]);
}

void Sim::dispatchCompute(const vk::raii::CommandBuffer &cmd, float deltaTime) {
    m_forwardRenderer->getDevice()->waitIdle();

    updateComputeDescriptors();

    uint32_t groupX = (m_window.WIDTH + 15) / 16;
    uint32_t groupY = (m_window.HEIGHT + 15) / 16;

    SimComputePushConstant pushData{};
    pushData.dt = deltaTime;
    pushData.dissipation = 0.99f;

    static double lastX = 0.0, lastY = 0.0;
    double xpos, ypos;
    glfwGetCursorPos(m_window.getWindow(), &xpos, &ypos);

    if (glfwGetMouseButton(m_window.getWindow(), GLFW_MOUSE_BUTTON_LEFT) ==
        GLFW_PRESS) {
        float mouseX = static_cast<float>(xpos / m_window.WIDTH);
        float mouseY = static_cast<float>(ypos / m_window.HEIGHT);

        float dx = static_cast<float>(xpos - lastX);
        float dy = static_cast<float>(ypos - lastY);

        pushData.point = glm::vec2(mouseX, mouseY);
        pushData.radius = 0.001f;

        m_splatPipeline->bind(cmd);

        pushData.color = glm::vec3(dx * 1.0f, dy * 1.0f, 0.0f);
        cmd.pushConstants<SimComputePushConstant>(
            *m_computePipelineLayout, vk::ShaderStageFlagBits::eCompute, 0,
            pushData);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                               *m_computePipelineLayout, 0,
                               {*m_computeDescriptorSets[6]}, nullptr);
        cmd.dispatch(groupX, groupY, 1);

        insertComputeBarrier(cmd, m_velocityTex[m_readIndex]->getImage());

        pushData.color = glm::vec3(0.1f);
        cmd.pushConstants<SimComputePushConstant>(
            *m_computePipelineLayout, vk::ShaderStageFlagBits::eCompute, 0,
            pushData);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                               *m_computePipelineLayout, 0,
                               {*m_computeDescriptorSets[7]}, nullptr);
        cmd.dispatch(groupX, groupY, 1);

        insertComputeBarrier(cmd, m_densityTex[m_readIndex]->getImage());
    }

    lastX = xpos;
    lastY = ypos;

    m_advectPipeline->bind(cmd);
    pushData.dissipation = 0.9999f;
    cmd.pushConstants<SimComputePushConstant>(*m_computePipelineLayout,
                                              vk::ShaderStageFlagBits::eCompute,
                                              0, pushData);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                           *m_computePipelineLayout, 0,
                           {*m_computeDescriptorSets[0]}, nullptr);
    cmd.dispatch(groupX, groupY, 1);
    insertComputeBarrier(cmd, m_velocityTex[m_writeIndex]->getImage());

    pushData.dissipation = 0.999f;
    cmd.pushConstants<SimComputePushConstant>(*m_computePipelineLayout,
                                              vk::ShaderStageFlagBits::eCompute,
                                              0, pushData);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                           *m_computePipelineLayout, 0,
                           {*m_computeDescriptorSets[1]}, nullptr);
    cmd.dispatch(groupX, groupY, 1);
    insertComputeBarrier(cmd, m_densityTex[m_writeIndex]->getImage());

    m_divergencePipeline->bind(cmd);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                           *m_computePipelineLayout, 0,
                           {*m_computeDescriptorSets[2]}, nullptr);
    cmd.dispatch(groupX, groupY, 1);
    insertComputeBarrier(cmd, m_divergenceTex->getImage());

    m_jacobiPipeline->bind(cmd);
    pushData.alpha = -1.0f;
    pushData.beta = 4.0f;
    cmd.pushConstants<SimComputePushConstant>(*m_computePipelineLayout,
                                              vk::ShaderStageFlagBits::eCompute,
                                              0, pushData);

    for (int i = 0; i < 20; i++) {
        int setIdx = (i % 2 == 0) ? 3 : 4;
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                               *m_computePipelineLayout, 0,
                               {*m_computeDescriptorSets[setIdx]}, nullptr);
        cmd.dispatch(groupX, groupY, 1);
        insertComputeBarrier(cmd, m_pressureTex[(i + 1) % 2]->getImage());
    }

    m_projectPipeline->bind(cmd);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                           *m_computePipelineLayout, 0,
                           {*m_computeDescriptorSets[5]}, nullptr);
    cmd.dispatch(groupX, groupY, 1);
    insertComputeBarrier(cmd, m_velocityTex[m_writeIndex]->getImage());

    std::swap(m_readIndex, m_writeIndex);
}

void Sim::insertComputeBarrier(const vk::raii::CommandBuffer &cmd,
                               const vk::raii::Image &image) {
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::eGeneral;
    barrier.newLayout = vk::ImageLayout::eGeneral;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.setImage(*image);
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eComputeShader |
                            vk::PipelineStageFlagBits::eFragmentShader,
                        vk::DependencyFlags(), nullptr, nullptr, barrier);
}

void Sim::onUpdate(float deltaTime) {
    if (glfwGetKey(m_window.m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        close();
    }

    float frameTimeMs = deltaTime * 1000.0f;

    m_frameTimes[m_frameTimeIndex] = frameTimeMs;
    m_frameTimeIndex = (m_frameTimeIndex + 1) % FRAME_HISTORY_COUNT;

    m_averageLatency = 0.0f;
    m_maxLatency = 0.0f;
    m_minLatency = 999.0f;
    for (float t : m_frameTimes) {
        m_averageLatency += t;
        if (t > m_maxLatency)
            m_maxLatency = t;
        if (t < m_minLatency && t > 0.0f)
            m_minLatency = t;
    }
    m_averageLatency /= FRAME_HISTORY_COUNT;

    float fps = (m_averageLatency > 0.0f) ? (1000.0f / m_averageLatency) : 0.0f;

    m_imguiLayer->beginFrame();

    ImGui::Begin("Engine Diagnostics");

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Performance Metrics");
    ImGui::Separator();

    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Avg Latency: %.3f ms", m_averageLatency);
    ImGui::Text("Min Latency: %.3f ms", m_minLatency);
    ImGui::Text("Max Latency (Spike): %.3f ms", m_maxLatency);

    ImGui::Spacing();

    char overlay[32];
    snprintf(overlay, sizeof(overlay), "Inst: %.2f ms", frameTimeMs);
    ImGui::PlotLines("##FrameTimes", m_frameTimes.data(), m_frameTimes.size(),
                     m_frameTimeIndex, overlay, 0.0f, 10.0f, ImVec2(0, 80));

    ImGui::End();
}

void Sim::onRender(Ghost::FrameInfo &frameInfo) {
    if (!m_quad || !m_fluidMaterial)
        return;

    Ghost::GhostRenderObject renderObj{};
    renderObj.mesh = m_quad;
    renderObj.material = m_fluidMaterial;
    renderObj.transformMatrix = glm::mat4(1.0f);

    std::vector<Ghost::GhostRenderObject> renderObjects = {renderObj};
    m_forwardRenderer->renderScene(frameInfo.commandBuffer, renderObjects);

	m_imguiLayer->render(frameInfo.commandBuffer);
}

void Sim::onShutdown() { m_forwardRenderer->getDevice()->waitIdle(); }

void Sim::close() { m_isRunning = false; }

} // namespace FluidSim
