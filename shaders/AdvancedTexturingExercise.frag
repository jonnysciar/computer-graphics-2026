#version 450
#extension GL_ARB_separate_shader_objects : enable

// ============================================================
//  FRAGMENT SHADER — ADVANCED TEXTURING EXERCISE (E07)
// ============================================================

// ------------------------------------------------------------
//  INPUT / OUTPUT
// ------------------------------------------------------------
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;  

layout(location = 0) out vec4 outColor;

// ------------------------------------------------------------
//  TEXTURE MAPS
// ------------------------------------------------------------
layout(binding = 1, set = 1) uniform sampler2D albedoMap[4];
layout(binding = 2, set = 1) uniform sampler2D normalMap[4];
layout(binding = 3, set = 1) uniform sampler2D metallicMap[4];
layout(binding = 4, set = 1) uniform sampler2D roughnessMap[4];
layout(binding = 5, set = 1) uniform sampler2D aoMap[4];


// ------------------------------------------------------------
//  UNIFORMS
// ------------------------------------------------------------
layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {

    // --- Directional light ---
    vec3 lightDir;
    vec4 lightColor;

    // --- Camera ---
    vec3 eyePos;

    // --- Hemispheric ambient ---
    vec4 ambientUpper;  // xyz = sky / upper  color  (lU)
    vec4 ambientLower;  // xyz = ground / lower color (lD)
    vec4 ambientDir;    // xyz = "up" direction for blending (d)

    // --- Debug ---
    vec4 debugView;     // z = texture index (0–3)

} gubo;


const float PI = 3.14159265359;


// ---- [TODO 1] Build the TBN frame matrix ------------------
mat3 computeTBN(vec3 N, vec3 T, float tangentW) {
    return mat3(1.0);   // replace
}


// ---- [TODO 2] Decode a tangent-space normal map -----------
vec3 getNormalFromMap(mat3 TBN, int ti) {
    return vec3(0.0, 0.0, 1.0);   // replace
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

// ---- [TODO 3] Compute base reflectivity F0 ----------------
vec3 computeF0(vec3 albedo, float metallic) {
    return vec3(0.04);   // replace
}

// ---- [TODO 4] Hemispheric ambient lighting ----------------
vec3 hemisphericAmbient(vec3 N, vec3 mA) {
    vec3 lU = gubo.ambientUpper.xyz;
    vec3 lD = gubo.ambientLower.xyz;
    vec3 d  = normalize(gubo.ambientDir.xyz);

    return vec3(0.0);   // replace
}


// ============================================================
//  MAIN
// ============================================================
void main() {
    int ti = int(gubo.debugView.z);

    // ---- Sample PBR textures -----------------------------------
    //  Albedo: textures are stored in sRGB, but all PBR
    //  math must be performed in linear space. Convert with:
    //  albedo_linear = pow(albedo_sRGB, vec3(2.2))
    vec3  albedo    = pow(texture(albedoMap[ti], fragUV).rgb, vec3(2.2));
    float metallic  = texture(metallicMap[ti], fragUV).r;
    float roughness = texture(roughnessMap[ti], fragUV).r;
    float ao        = texture(aoMap[ti], fragUV).r;

    // ---- Build TBN and decode the normal map ------
    mat3 TBN = computeTBN(fragNorm, fragTan.xyz, fragTan.w);
    vec3 N   = getNormalFromMap(TBN, ti);

    // ---- Geometry setup ---------------------
    vec3 V = normalize(gubo.eyePos - fragPos);
    vec3 L = normalize(-gubo.lightDir);
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

    // ---- [TODO 5a] Assemble the full PBR BRDF ------------------
    vec3 fr = f_diffuse + f_specular;   // replace

    vec3 direct = gubo.lightColor.rgb * fr;

    // ---- [TODO 5b] Apply Ambient Occlusion ---------------------
    vec3 ambient = hemisphericAmbient(N, albedo);   // add AO attenuation

    vec3 color = direct + ambient;

    // ---- Tone mapping and gamma correction ------------
    //
    //  Direct and ambient radiance values can exceed [0,1].
    //  Two steps are needed before writing to the framebuffer:
    //
    //  Step 1 — Reinhard tone mapping (maps HDR → [0,1]):
    //    color = color / (color + vec3(1.0))
    //
    //  Step 2 — Gamma correction (linear → sRGB):
    //    color = pow(color, vec3(1.0 / 2.2))
    //
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color, 1.0);
}
