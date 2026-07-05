#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec2 fragUV;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    fragUV = inUV;
}
