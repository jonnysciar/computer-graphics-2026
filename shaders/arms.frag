#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;

layout(location = 0) out vec4 outColor;

#define NUM_LIGHTS 19

layout(binding = 1, set = 0) uniform GlobalUniformBufferObject {
    vec4 lightColor[NUM_LIGHTS];   // rgb = colour of light i
    vec4 lightPos[NUM_LIGHTS];     // xyz = world position of light i
    vec4 lightParams[NUM_LIGHTS];  // x = on/off, y = g, z = beta
    vec4 playerPos;
    vec4 extra;
} gubo;

layout(binding = 2, set = 0) uniform sampler2D textureMap[2];

void main() {
    float uv = texture(textureMap[1], fragUV).r;
    vec3 albedo = pow(texture(textureMap[0], fragUV).rgb, vec3(2.2));
    outColor = vec4(albedo, 1.0);
}
