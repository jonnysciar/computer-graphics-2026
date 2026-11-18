#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out float fragDepth;



void main() {
	gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);

    fragDepth = gl_Position.w;
	fragUV = inUV;
}
