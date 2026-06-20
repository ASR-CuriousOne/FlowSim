#include <Ghost/Resources/material.hpp>

namespace Ghost {
Material::Material(std::shared_ptr<GhostGraphicsPipeline> pipeline,
                   uint32_t pipelineTypeId)
    : m_pipeline(std::move(pipeline)), m_pipelineTypeId(pipelineTypeId) {}
} // namespace Ghost
