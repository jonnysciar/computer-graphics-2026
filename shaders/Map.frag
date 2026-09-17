#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (binding = 0, set = 0) uniform MakeMapUniformBufferObject {
	mat4 Transform;
	vec4 color;
} mmubo;

void main() {
	outColor = mmubo.color;
}