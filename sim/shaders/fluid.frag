#version 450

layout(location = 0) in vec2 fragUV;

layout(binding = 0) uniform sampler2D u_Velocity;
layout(binding = 1) uniform sampler2D u_Density;

layout(location = 0) out vec4 outColor;

vec3 cosinePalette(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d) {
    return a + b * cos(6.28318530718 * (c * t + d));
}

void main() {
    vec2 vel = texture(u_Velocity, fragUV).xy;
    float den = texture(u_Density, fragUV).x;

    float angle = atan(vel.y, vel.x);
    float normalizedAngle = (angle + 3.14159265359) / (2.0 * 3.14159265359);

    vec3 a = vec3(0.5, 0.5, 0.5);       
    vec3 b = vec3(0.5, 0.5, 0.5);       
    vec3 c = vec3(1.0, 1.0, 1.0);       
    vec3 d = vec3(0.00, 0.33, 0.67);    
    
    vec3 baseColor = cosinePalette(normalizedAngle, a, b, c, d);

    float mask = smoothstep(0.0, 2.0, den); 
    
    float speed = length(vel);
    float dynamicBrightness = 0.5 + (0.5 * smoothstep(0.0, 15.0, speed));
    
    vec3 finalColor = baseColor * mask * dynamicBrightness;

    finalColor = pow(finalColor, vec3(1.2)); 

    outColor = vec4(finalColor, 1.0);
}
