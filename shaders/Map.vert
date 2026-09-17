#version 450
#extension GL_ARB_separate_shader_objects : enable

// even if normal is not required, adding it so to use a single vertex type, simplifies a lot
// the application. I think in this case is a good trade-off between efficiency and
// compactness.
layout (location = 0) in vec3 inPos;

layout (binding = 0, set = 0) uniform MakeMapUniformBufferObject {
	mat4 Transform;
	vec4 color;
} mmubo;

void main() {
    gl_Position = vec4((mmubo.Transform * vec4(inPos, 1.0f)).xy, 0.0f, 1.0f);
}
