#include <Ghost/Systems/ghostRenderer.hpp>
#include <Ghost/Utils/utils.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
GhostRenderer::GhostRenderer(VulkanDevice &device, vk::SurfaceKHR surface,
                             uint32_t width, uint32_t height)
    : m_device(device), m_extent(width, height), m_surface(surface),
      m_commandPool(device) {

    m_swapchain =
        std::make_unique<GhostSwapchain>(m_device, m_surface, m_extent);

    createRenderPass();
    createFramebuffers();

    createCommandBuffers();
    createSyncObjects();
}

GhostRenderer::~GhostRenderer() {}

void GhostRenderer::createCommandBuffers() {
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandPool(*m_commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(MAX_FRAMES_IN_FLIGHT);

    m_commandBuffers = vk::raii::CommandBuffers(m_device, allocInfo);
}

void GhostRenderer::createSyncObjects() {
    m_imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_imageAvailableSemaphores.emplace_back(m_device, semaphoreInfo);
        m_inFlightFences.emplace_back(m_device, fenceInfo);
    }

    size_t imageCount = m_swapchain->getImageCount();
    m_renderFinishedSemaphores.reserve(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        m_renderFinishedSemaphores.emplace_back(m_device, semaphoreInfo);
    }
}

void GhostRenderer::createRenderPass() {
    vk::AttachmentDescription colorAttachment;

    colorAttachment.setFormat(m_swapchain->getSwapchainImageFormat())
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setAttachment(0);
    colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription depthAttachment{};
    depthAttachment.setFormat(m_device.findDepthFormat())
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.setAttachment(1).setLayout(
        vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1)
        .setColorAttachments(colorAttachmentRef);
    subpass.setPDepthStencilAttachment(&depthAttachmentRef);

    vk::SubpassDependency dependency;

    dependency.setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                         vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                         vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                          vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    vk::RenderPassCreateInfo renderPassInfo;

    std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment,
                                                            depthAttachment};

    renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
        .setPAttachments(attachments.data())
        .setSubpassCount(1)
        .setSubpasses(subpass)
        .setDependencyCount(1)
        .setDependencies(dependency);

    m_renderPass = vk::raii::RenderPass(m_device, renderPassInfo);
}

void GhostRenderer::createFramebuffers() {
    m_framebuffers.clear();
    m_framebuffers.reserve(m_swapchain->getImageCount());

    for (size_t i = 0; i < m_swapchain->getImageCount(); i++) {
        std::array<vk::ImageView, 2> attachments = {
            *m_swapchain->getImageView(i), *m_swapchain->getDepthImageView(i)};

        vk::FramebufferCreateInfo createInfo;
        createInfo.setRenderPass(*m_renderPass)
            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
            .setPAttachments(attachments.data())
            .setWidth(m_swapchain->getSwapchainExtent().width)
            .setHeight(m_swapchain->getSwapchainExtent().height)
            .setLayers(1);

        m_framebuffers.emplace_back(
            vk::raii::Framebuffer(m_device, createInfo));
    }
}

void GhostRenderer::flagFramebufferResized(uint32_t width, uint32_t height) {
    m_extent.width = width;
    m_extent.height = height;
    m_framebufferResized = true;
}

void GhostRenderer::recreateSwapchain() {
    if (m_extent.width == 0 || m_extent.height == 0)
        return;

    m_device->waitIdle();
    m_swapchain.reset();

    m_swapchain =
        std::make_unique<GhostSwapchain>(m_device, m_surface, m_extent);
    createFramebuffers();
}

vk::raii::CommandBuffer &GhostRenderer::beginFrame() {
    assert(!m_isFrameStarted &&
           "Can't call beginFrame while already in progress");

    m_device->waitForFences({*m_inFlightFences[m_currentFrame]}, vk::True,
                            std::numeric_limits<uint64_t>::max());

    try {
        auto resultValue = m_swapchain->aquireNextImage(
            m_imageAvailableSemaphores[m_currentFrame]);
        m_currentImageIndex = resultValue.value;
    } catch (const vk::OutOfDateKHRError &e) {
        recreateSwapchain();
        return m_commandBuffers[m_currentFrame];
    }

    m_isFrameStarted = true;
    m_device->resetFences({*m_inFlightFences[m_currentFrame]});

    auto &commandBuffer = m_commandBuffers[m_currentFrame];
    commandBuffer.reset();

    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void GhostRenderer::endFrame() {
    assert(m_isFrameStarted &&
           "Can't call endFrame while frame is not in progress");
    auto &commandBuffer = m_commandBuffers[m_currentFrame];
    commandBuffer.end();

    vk::PipelineStageFlags waitStages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::Semaphore waitSemaphores[] = {
        *m_imageAvailableSemaphores[m_currentFrame]};
    vk::Semaphore signalSemaphores[] = {
        *m_renderFinishedSemaphores[m_currentImageIndex]};

    vk::SubmitInfo submitInfo(1, waitSemaphores, waitStages, 1,
                              &(*m_commandBuffers[m_currentFrame]), 1,
                              signalSemaphores);

    m_device.submitGraphicsQueue(submitInfo, m_inFlightFences[m_currentFrame]);

    vk::SwapchainKHR swapchains[] = {**m_swapchain};
    vk::PresentInfoKHR presentInfo(1, signalSemaphores, 1, swapchains,
                                   &m_currentImageIndex);

    try {
        vk::Result result = m_device.submitPresentQueue(presentInfo);

        if (result == vk::Result::eSuboptimalKHR || m_framebufferResized) {
            m_framebufferResized = false;
            recreateSwapchain();
        }
    } catch (const vk::OutOfDateKHRError &e) {
        m_framebufferResized = false;
        recreateSwapchain();
    }

    m_isFrameStarted = false;
    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void GhostRenderer::beginSwapChainRenderPass(
    const vk::raii::CommandBuffer &commandBuffer) {
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.setRenderPass(m_renderPass)
        .setFramebuffer(m_framebuffers[m_currentImageIndex]);
    renderPassInfo.renderArea.offset.setX(0.0f).setY(0.0f);
    renderPassInfo.renderArea.setExtent(m_swapchain->getSwapchainExtent());

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color =
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()))
        .setPClearValues(clearValues.data());

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.setX(0.0f)
        .setY(0.0f)
        .setWidth(m_swapchain->getSwapchainExtent().width)
        .setHeight(m_swapchain->getSwapchainExtent().height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor;
    scissor.offset.setX(0.0f).setY(0.0f);
    scissor.setExtent(m_swapchain->getSwapchainExtent());
    commandBuffer.setScissor(0, scissor);
}

void GhostRenderer::endSwapChainRenderPass(
    const vk::raii::CommandBuffer &commandBuffer) {
    commandBuffer.endRenderPass();
}

} // namespace Ghost
