#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform ShadowUBO {
	mat4 faceVP[6];
	vec4 lightPosFar;          // xyz = light position, w = far plane
	mat4 model;
} subo;

layout(push_constant) uniform PC {
	uint face;                 // cube face being rendered (0..5)
} pc;

layout(location = 0) in vec3 inPosition;   // other vertex attributes are ignored

layout(location = 0) out vec3 fragWorldPos;

void main() {
	vec4 wp = subo.model * vec4(inPosition, 1.0);
	fragWorldPos = wp.xyz;
	gl_Position = subo.faceVP[pc.face] * wp;
}
