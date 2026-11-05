#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform ShadowUBO {
	mat4 faceVP[6];
	vec4 lightPosFar;
	mat4 model;
} shadow;

layout(location = 0) in vec3 fragWorldPos;

layout(location = 0) out float outDistance;   // R32_SFLOAT cube face

void main() {
	// world-space distance light -> surface (the scene shader compares against it)
	outDistance = length(fragWorldPos - shadow.lightPosFar.xyz);
}
