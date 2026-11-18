#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragUV;
layout(location = 1) in float fragDepth;

layout(location = 0) out vec4 outColor;

layout(binding = 1, set = 0) uniform sampler2D textureMap;

const float ARMS_CUT_DISTANCE = 0.6;

void main() {
    if (fragDepth < ARMS_CUT_DISTANCE) discard;

    vec3 albedo = texture(textureMap, fragUV).rgb;

    outColor = vec4(albedo, texture(textureMap, fragUV).a) * 0.5;
}
