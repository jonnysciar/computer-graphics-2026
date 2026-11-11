#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;

layout(location = 0) out vec4 outColor;

#define NUM_LIGHTS 8

layout(binding = 1, set = 0) uniform GlobalUniformBufferObject {
    vec4 lightColor[NUM_LIGHTS];   // rgb = colour of light i
    vec4 lightPos[NUM_LIGHTS];     // xyz = world position of light i
    vec4 lightParams[NUM_LIGHTS];  // x = on/off, y = g, z = beta
    vec4 playerPos;
    vec4 extra;
} gubo;

layout(binding = 2, set = 0) uniform sampler2D tex;

// 0 = flat light color, 1 = faces turned to the viewer become white
#define WHITE_CORE 0

// Unlit: the model glows with its light color and ignores the light it contains
void main() {
    vec3 N = normalize(fragNorm);
    vec3 V = normalize(gubo.playerPos.xyz - fragPos);
    float facing = max(dot(N, V), 0.0);

    vec3 texColor = texture(tex, fragUV).rgb;
    vec3 color = mix(gubo.extra.rgb, vec3(1.0), WHITE_CORE * facing * facing) * gubo.extra.a * texColor;
    outColor = vec4(color, 1.0);
}
