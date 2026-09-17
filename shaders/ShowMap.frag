#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (location = 0) in vec2 fragUV;

layout (binding = 0, set = 0) uniform sampler2D tex;

void main() {
	outColor = texture(tex, fragUV);
}