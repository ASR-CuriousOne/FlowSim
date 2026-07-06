#pragma once
#include <Ghost/Core/ghostComputePipeline.hpp>
#include <Ghost/Core/ghostGraphicsPipeline.hpp>
#include <Ghost/Resources/ghostDescriptorManager.hpp>
#include <Ghost/Resources/material.hpp>
#include <Ghost/Resources/mesh.hpp>
#include <Ghost/Resources/storageTexture.hpp>
#include <Ghost/Systems/forwardRenderer.hpp>
#include <Ghost/Utils/frameInfo.hpp>
#include <Sim/fluidMaterial.hpp>
#include <Sim/imguiLayer.hpp>
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

    void createPipeline();
    void generateQuad();
    void createMaterial();

    void createComputePipeline();
    void createComputeResources();

    void updateComputeDescriptors();
    void dispatchCompute(const vk::raii::CommandBuffer &cmd, float deltaTime);
    void insertComputeBarrier(const vk::raii::CommandBuffer &cmd,
                              const vk::raii::Image &image);
	void insertComputeToGraphicsBarrier(const vk::raii::CommandBuffer &cmd,
                              const vk::raii::Image &image);

    WindowGLFW m_window;
    std::unique_ptr<Ghost::ForwardRenderer> m_forwardRenderer;
    std::unique_ptr<Ghost::GhostDescriptorManager> m_descriptorManager;

    std::unique_ptr<ImGuiLayer> m_imguiLayer;

    std::shared_ptr<Ghost::StorageTexture> m_velocityTex[2];
    std::shared_ptr<Ghost::StorageTexture> m_densityTex[2];
    std::shared_ptr<Ghost::StorageTexture> m_pressureTex[2];
    std::shared_ptr<Ghost::StorageTexture> m_divergenceTex;

    vk::raii::PipelineLayout m_computePipelineLayout = nullptr;
    std::shared_ptr<Ghost::GhostComputePipeline> m_advectPipeline;
    std::shared_ptr<Ghost::GhostComputePipeline> m_divergencePipeline;
    std::shared_ptr<Ghost::GhostComputePipeline> m_jacobiPipeline;
    std::shared_ptr<Ghost::GhostComputePipeline> m_projectPipeline;
    std::shared_ptr<Ghost::GhostComputePipeline> m_splatPipeline;

    std::vector<vk::raii::DescriptorSet> m_computeDescriptorSets[2];

    std::shared_ptr<Ghost::Mesh> m_quad;
    vk::raii::PipelineLayout m_fluidGraphicsPipelineLayout = nullptr;
    std::shared_ptr<Ghost::GhostGraphicsPipeline> m_fluidGraphicsPipeline;
    std::shared_ptr<FluidMaterial> m_fluidMaterial;

    uint32_t m_readIndex = 0;
    uint32_t m_writeIndex = 1;
    bool m_isRunning = true;

    static constexpr size_t FRAME_HISTORY_COUNT = 1000;
    std::array<float, FRAME_HISTORY_COUNT> m_frameTimes{};
    size_t m_frameTimeIndex = 0;
    float m_averageLatency = 0.0f;
    float m_maxLatency = 0.0f;
    float m_minLatency = 999.0f;
};
} // namespace FluidSim
