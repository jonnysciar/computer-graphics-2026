#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_SHADOW_LIGHTS 8        // = ShadowMapper::MaxLights

layout(set = 0, binding = 0) uniform ShadowUBO {
	mat4 model;                    // caster world matrix
	vec4 params;                   // x = near, y = far, z = light count
	vec4 lightPos[MAX_SHADOW_LIGHTS];
} ubo;

layout(push_constant) uniform PC {
	uint layer;                    // light * 6 + face
} pc;

layout(location = 0) in vec3 inPosition;   // other vertex attributes are ignored
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) flat out vec3 fragLightPos;

// Cube face basis, derived from the Vulkan cube-map face selection table
// (face order +X -X +Y -Y +Z -Z).  x = dot(R, d), y = dot(U, d), depth = dot(F, d)
const vec3 R[6] = vec3[](vec3(0,0,-1), vec3(0,0,1), vec3(1,0,0), vec3(1,0,0), vec3(1,0,0), vec3(-1,0,0));
const vec3 U[6] = vec3[](vec3(0,-1,0), vec3(0,-1,0), vec3(0,0,1), vec3(0,0,-1), vec3(0,-1,0), vec3(0,-1,0));
const vec3 F[6] = vec3[](vec3(1,0,0), vec3(-1,0,0), vec3(0,1,0), vec3(0,-1,0), vec3(0,0,1), vec3(0,0,-1));

void main() {
	uint light = pc.layer / 6u;
	uint face  = pc.layer % 6u;
	vec3  L    = ubo.lightPos[light].xyz;
	float n    = ubo.params.x;
	float f    = ubo.params.y;

	vec4 wp = ubo.model * vec4(inPosition, 1.0);
	vec3 d  = wp.xyz - L;

	// 90 degree perspective, aspect 1, Vulkan depth range 0..1, no y flip needed
	float zf = dot(F[face], d);
	gl_Position = vec4(dot(R[face], d), dot(U[face], d),
	                   zf * (f / (f - n)) - f * n / (f - n), zf);

	fragWorldPos = wp.xyz;
	fragLightPos = L;
}
