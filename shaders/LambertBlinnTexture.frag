#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragNorm;
layout (location = 2) in vec2 fragUV;

// now we need to read the values in the uniforms
// in this shader, we need the local uniforms
layout (binding = 0, set = 1) uniform UniformBufferObject {
	mat4 vpMat;
	vec4 specular;
	vec3 ambient;
} ubo;

// the shaders now receives also the texture in a separate set
layout (binding = 1, set = 1) uniform sampler2D tex;

// and also the global
layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    vec3 lightDir;
    vec3 lightColor;
    vec3 eyePos;
	vec3 ambientLight;
} gubo;



void main() {
	// returns a color computed with lambert + blinn
	vec3 N = normalize(fragNorm);
	vec3 V = normalize(gubo.eyePos - fragPos);
	vec3 L = gubo.lightDir;
	
	N = gl_FrontFacing ? N : -N;
	
	// reads the color from the texture
	vec3 albedo = texture(tex, fragUV).rgb;

	// lambert diffuse
	float kD = max(dot(N, L), 0.0);

	// blinn specular
	vec3 H = normalize(V + L);
	float kS = pow(max(dot(H, N), 0.0), ubo.specular.w);
	
	// final color
	vec3 color = (kD * albedo + kS * ubo.specular.rgb) * gubo.lightColor * (1.0 - gubo.ambientLight) +
				 ubo.ambient * gubo.ambientLight * albedo;
	
	outColor = vec4(color, 1.0f);
}