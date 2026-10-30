#version 450
#extension GL_ARB_separate_shader_objects : enable

// ------------------------------------------------------------
//  INPUT / OUTPUT
// ------------------------------------------------------------
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;  

layout(location = 0) out vec4 outColor;

// ------------------------------------------------------------
//  UNIFORMS
// ------------------------------------------------------------
layout(binding = 1, set = 0) uniform GlobalUniformBufferObject {
    vec4 lightColor;
    vec3 lightParams;
    vec3 playerPos;
} gubo;

// ------------------------------------------------------------
//  TEXTURE MAPS
// ------------------------------------------------------------
layout(binding = 2, set = 0) uniform sampler2D textureMap[5];


const float PI = 3.14159265359;

vec3 pointLightColor(vec3 lPos, vec3 fragPosition, out vec3 L) {
    vec3  toLight = lPos - fragPosition;
    float dist    = max(length(toLight), 0.0001);
    L = toLight / dist;

    float g    = gubo.lightParams.x;
    float beta = gubo.lightParams.y;

    float decay = pow(g / dist, beta);
    return gubo.lightColor.rgb * decay;
}

mat3 computeTBN(vec3 N, vec3 T, float tangentW) {
    N = normalize(N);
    T = normalize(T - dot(T, N) * N);   // Gram-Schmidt re-orthogonalization
    vec3 B = cross(N, T) * tangentW;    // w holds the handedness (+1 / -1)
    return mat3(T, B, N);               // columns: tangent space -> world space
}

vec3 computeNormalMap(mat3 TBN) {
    vec3 n = texture(textureMap[1], fragUV).rgb * 2.0 - 1.0;
    return normalize(TBN * n);          // tangent space -> world space
}

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float dotVH, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - dotVH, 0.0, 1.0), 5.0);
}

// GGX Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2     = roughness * roughness;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = NdotH2 * (a2 - 1.0) + 1.0;
    denom = max(denom, 0.0001);
    return a2 / (PI * denom * denom);
}

// Schlick-GGX single-direction geometry term
float GeometrySchlickGGX(float NdotA, float roughness) {
    float k = (roughness + 1.0);
    k = (k * k) / 8.0;
    return NdotA / (NdotA * (1.0 - k) + k);
}

// Smith geometry function (combines view and light directions)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 computeF0(vec3 albedo, float metallic) {
    return mix(vec3(0.04), albedo, metallic);
}

vec3 computeLight(vec3 lPos) {
    // ---- Sample PBR textures -----------------------------------
    //  Albedo: textures are stored in sRGB, but all PBR
    //  math must be performed in linear space. Convert with:
    //  albedo_linear = pow(albedo_sRGB, vec3(2.2))
    vec3  albedo    = pow(texture(textureMap[0], fragUV).rgb, vec3(2.2));
    float metallic  = texture(textureMap[2], fragUV).r;
    float roughness = texture(textureMap[3], fragUV).r;
    float ao        = texture(textureMap[4], fragUV).r;

    // ---- Build TBN and decode the normal map ------
    mat3 TBN = computeTBN(fragNorm, fragTan.xyz, fragTan.w);
    vec3 N = normalize(computeNormalMap(TBN));

    // ---- Geometry setup ---------------------
    vec3 V = normalize(gubo.playerPos - fragPos);
    vec3 L;
    vec3 lightCol = pointLightColor(lPos, fragPos, L);      // fills L, returns attenuated color
    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    // ---- Cook-Torrance terms ----------------
    vec3  F0 = computeF0(albedo, metallic);
    vec3  F  = fresnelSchlick(max(dot(V, H), 0.0), F0);
    float D  = DistributionGGX(N, H, roughness);
    float G  = GeometrySmith(N, V, L, roughness);

    vec3 f_diffuse  = albedo * NdotL;
    vec3 f_specular = (D * G * F) / max(4.0 * NdotV, 0.001);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);   // energy conservation; metals have no diffuse
    vec3 fr = kD * f_diffuse + f_specular;

    //vec3 direct = gubo.lightColor.rgb * fr;
    vec3 direct = lightCol * fr;

    vec3 color = direct;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    return color;
}

// ============================================================
//  MAIN
// ============================================================
void main() {
    outColor = vec4(computeLight(vec3(4.0, 3.0, 4.0)), 1.0);
}
