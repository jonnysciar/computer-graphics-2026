#version 450
#extension GL_ARB_separate_shader_objects : enable

#define NUM_LIGHTS 8        //

layout(set = 0, binding = 0) uniform ShadowUBO {
	mat4 model;                    // caster world matrix
	vec4 params;                   // x = near, y = far, z = light count, w = guard band scale
	vec4 lightPos[NUM_LIGHTS];
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
	uint  light = pc.layer / 6u;
	uint  face  = pc.layer % 6u;
	vec3  L     = ubo.lightPos[light].xyz;
	float n     = ubo.params.x;
	float f     = ubo.params.y;
	float s     = ubo.params.w;    // < 1: slightly wider than 90 degrees (guard band for PCF)

	vec3 d  = (ubo.model * vec4(inPosition, 1.0)).xyz - L;

	// Perspective, aspect 1, Vulkan depth 0..1.  Stored depth = A + B / zf  (zf = distance along the face axis)
	float zf = dot(F[face], d);
	gl_Position = vec4(s * dot(R[face], d), s * dot(U[face], d),
	                   zf * (f / (f - n)) - f * n / (f - n), zf);
}
