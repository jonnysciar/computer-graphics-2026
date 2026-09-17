#version 450
#extension GL_ARB_separate_shader_objects : enable

// this time, positions and colors are sent by the application
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragNorm;
layout (location = 2) out vec2 fragUV;

// now we need to read the values in the uniforms
// in this shader, we need only the local uniforms
layout (binding = 0, set = 1) uniform UniformBufferObject {
	mat4 vpMat;
	vec4 specular;
	vec3 ambient;
} ubo;

layout( push_constant ) uniform constants
{
	mat4 mMat;
	mat4 uvMat;
} PushConstants;

void main() {
	// now the shader becomes more serious:
	// it computes the normalized screen coordinates with the world-view-projection matrix
	// it computes the world coordinates of the point with the world matrix
	// it computes the normal direction with the corresponding matrix
	mat4 mMat  = PushConstants.mMat;
	mat4 uvMat = PushConstants.uvMat;
    gl_Position = ubo.vpMat * mMat * vec4(inPos, 1.0f);
	fragPos     = (mMat * vec4(inPos , 1.0f)).xyz;
	fragNorm    = (mMat * vec4(inNorm, 0.0f)).xyz;
	fragUV		= (uvMat * vec4(inUV, 0.0f, 1.0f)).xy;
}
