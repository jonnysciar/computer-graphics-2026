#version 450
// Bake pass: rasterize the mesh INTO its lightmap by using UV2 as the clip-space position.
// Pipeline state: no culling (UV winding is arbitrary), no depth test, viewport = lightmap size.
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 4) in vec2 inUV2;

layout(push_constant) uniform PC { mat4 model; } pc; // if your vertices are already in world space, drop this

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;

void main() {
    vec4 wp = pc.model * vec4(inPos, 1.0);
    fragPos = wp.xyz;
    fragNorm = transpose(inverse(mat3(pc.model))) * inNorm;
    // Vulkan: clip (-1,-1) is the framebuffer's top-left, same as uv (0,0) -> no flip needed
    gl_Position = vec4(inUV2 * 2.0 - 1.0, 0.0, 1.0);
}
