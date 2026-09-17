#version 450
#extension GL_ARB_separate_shader_objects : enable

// even if normal is not required, adding it so to use a single vertex type, simplifies a lot
// the application. I think in this case is a good trade-off between efficiency and
// compactness.
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;


layout (location = 0) out vec2 fragUV;


void main() {
    gl_Position = vec4(inPos, 1.0f);
	fragUV		= inUV;
}
