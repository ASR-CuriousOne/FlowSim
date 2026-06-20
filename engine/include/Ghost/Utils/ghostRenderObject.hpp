#pragma once
#include <Ghost/Resources/material.hpp>
#include <Ghost/Resources/mesh.hpp>
#include <glm/glm.hpp>

namespace Ghost {
struct GhostRenderObject {
    glm::mat4 transformMatrix{1.f};
    std::shared_ptr<Mesh> mesh{};
    std::shared_ptr<Material> material{};
};
} // namespace Ghost
