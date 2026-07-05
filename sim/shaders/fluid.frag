#version 450

layout(location = 0) in vec2 fragUV;

layout(binding = 0) uniform sampler2D u_Velocity;
layout(binding = 1) uniform sampler2D u_Density;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 vel = texture(u_Velocity, fragUV).xy;
    float den = texture(u_Density, fragUV).x;
    
    float speed = length(vel);

    vec3 bgDark      = vec3(0.0,0.0,0.0); 
    vec3 neonCyan    = vec3(0.490, 0.812, 1.000);
    vec3 neonPurple  = vec3(0.733, 0.604, 0.969);
    vec3 neonPink    = vec3(0.969, 0.463, 0.557);

    vec3 fluidColor = mix(bgDark, neonCyan, clamp(den, 0.0, 1.0));
    fluidColor = mix(fluidColor, neonPurple, clamp(den - 1.0, 0.0, 1.0));

    float speedFactor = smoothstep(0.0, 30.0, speed); 
    vec3 finalColor = mix(fluidColor, neonPink, speedFactor);

    float angle = atan(vel.y, vel.x);
    vec3 directionalTint = vec3(cos(angle), sin(angle), -cos(angle));
    
    finalColor += 0.15 * directionalTint * speedFactor;

    outColor = vec4(finalColor, 1.0);
}
