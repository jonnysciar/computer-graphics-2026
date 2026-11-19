#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;

layout(location = 0) out vec4 outColor;

layout(binding = 1, set = 0) uniform LightUniformBufferObject {
    vec4 lightColor;   // rgb = colour of light i, a = on/off
    vec4 playerPos;
} lubo;

layout(binding = 2, set = 0) uniform sampler2D textureMap[2];

// 0 = flat light color, 1 = faces turned to the viewer become white
#define WHITE_CORE 0

// Unlit: the model glows with its light color and ignores the light it contains
void main() {
    vec3 N = normalize(fragNorm);
    vec3 V = normalize(lubo.playerPos.xyz - fragPos);
    float facing = max(dot(N, V), 0.0);

    vec3 texColor = texture(textureMap[0], fragUV).rgb;
    float alpha = texture(textureMap[1], fragUV).r * lubo.lightColor.a;
    vec3 color = mix(lubo.lightColor.rgb, vec3(1.0), WHITE_CORE * facing * facing) * texColor;
    outColor = vec4(color, alpha);
}
