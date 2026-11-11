#version 450
#extension GL_ARB_separate_shader_objects : enable

// Depth-only pass: the depth buffer is the shadow map, nothing to write.
void main() { }
