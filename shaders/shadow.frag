#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) flat in vec3 fragLightPos;

layout(location = 0) out float outDistance;   // R32_SFLOAT layer

void main() {
	// world-space distance light -> surface (the scene shader compares against it)
	outDistance = length(fragWorldPos - fragLightPos);
}
