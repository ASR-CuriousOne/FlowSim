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

void Sim::onInit() {}

void Sim::onUpdate(float deltaTime) {}

void Sim::onRender(Ghost::FrameInfo &frameInfo) {}

void Sim::onShutdown() {}

void Sim::close() { m_isRunning = false; }

} // namespace FluidSim
