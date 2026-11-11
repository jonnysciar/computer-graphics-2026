#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    vec4 emission; // rgb = light color, a = glow strength
} ubo;

#define MAX_LIGHTS 64

struct PointLight {
    vec4 posRange;
    vec4 colorInt;
};

layout(binding = 1, set = 0) uniform GlobalUniformBufferObject {
    vec3 playerPos;
    int lightCount;
    vec4 decay;
    PointLight lights[MAX_LIGHTS];
} gubo;

// 0 = flat light color, 1 = faces turned to the viewer become white
#define WHITE_CORE 0

// Unlit: the model glows with its light color and ignores the light it contains
void main() {
    vec3 N = normalize(fragNorm);
    vec3 V = normalize(gubo.playerPos - fragPos);
    float facing = max(dot(N, V), 0.0);

    vec3 color = mix(ubo.emission.rgb, vec3(1.0), WHITE_CORE * facing * facing) * ubo.emission.a;
    outColor = vec4(color, 1.0);
}
