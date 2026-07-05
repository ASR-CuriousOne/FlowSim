#pragma once
#include <glm/glm.hpp>

namespace FluidSim {

struct SimGraphicsPushConstant {
    float dt;
    float dissipation;
    float alpha;
    float beta;
};

struct SimComputePushConstant {
    float dt;
    float dissipation;
    float alpha;
    float beta;
    alignas(8) glm::vec2 point;
    alignas(16) glm::vec3 color;
    float radius;
};

} // namespace FluidSim
