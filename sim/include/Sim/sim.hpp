#pragma once
#include <Ghost/Resources/ghostDescriptorManager.hpp>
#include <Ghost/Systems/forwardRenderer.hpp>
#include <Ghost/Utils/frameInfo.hpp>
#include <Sim/windowGLFW.hpp>

namespace FluidSim {
class Sim {
  public:
    Sim();
    ~Sim() = default;

    void run();
    void close();

  protected:
    void onInit();
    void onUpdate(float deltaTime);
    void onRender(Ghost::FrameInfo &frameInfo);
    void onShutdown();

    WindowGLFW m_window;

    std::unique_ptr<Ghost::ForwardRenderer> m_forwardRenderer;
    std::unique_ptr<Ghost::GhostDescriptorManager> m_descriptorManager;

  private:
    bool m_isRunning = true;
};
} // namespace FluidSim
