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
#define NUM_LIGHTS 19        // = ShadowMapperParams::lightCount (light i <-> shadow layers 6i..6i+5)

layout(binding = 1, set = 0) uniform GlobalUniformBufferObject {
    vec4 lightColor[NUM_LIGHTS]; // rgb = colour of light i
    vec4 lightPos[NUM_LIGHTS];   // xyz = world position of light i
    vec4 lightParams[NUM_LIGHTS];// x = on/off, y = g, z = beta, w = NEW: squared influence radius (0 = unbounded), computed on the CPU
    vec4 playerPos;
    vec4 settings; //x = enable shadows, y = shading technique, z = num of usable lights
} gubo;

// ------------------------------------------------------------
//  TEXTURE MAPS
// ------------------------------------------------------------
layout(binding = 2, set = 0) uniform sampler2D textureMap[5];
layout(binding = 3, set = 0) uniform sampler2DArrayShadow shadowMaps;

const float PI = 3.14159265359;

mat3 computeTBN(vec3 N, vec3 T, float tangentW) {
    N = normalize(N);
    T = normalize(T - dot(T, N) * N); // Gram-Schmidt re-orthogonalization
    vec3 B = cross(N, T) * tangentW;  // w holds the handedness (+1 / -1)
    return mat3(T, B, N);             // columns: tangent space -> world space
}

vec3 computeNormalMap(mat3 TBN) {
    vec3 n = texture(textureMap[1], fragUV).rgb * 2.0 - 1.0;
    return normalize(TBN * n);
}

vec3 computeF0(vec3 albedo, float metallic) {
    return mix(vec3(0.04), albedo, metallic);
}

// pow(x, 5) without the pow() (x already in [0,1])
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

// ------------------------------------------------------------
//  OMNIDIRECTIONAL SHADOWS (hardware PCF)
// ------------------------------------------------------------
// These three MUST match the C++ side
#define SHADOW_NEAR          0.1    // = ShadowMapperParams::nearPlane
#define SHADOW_FAR           100.0  // = ShadowMapperParams::farPlane
#define SHADOW_GUARD_TEXELS  4.0    // = ShadowMapper::GuardTexels

const float SHADOW_BIAS = 0.05;
const float SHADOW_NORMAL_OFFSET = 0.05;
const float LIGHT_CUTOFF = 1e-4; // lights contributing less than this are skipped

const float SHADOW_A = SHADOW_FAR / (SHADOW_FAR - SHADOW_NEAR);
const float SHADOW_B = -SHADOW_FAR * SHADOW_NEAR / (SHADOW_FAR - SHADOW_NEAR);

// uvScale = 0.5 * guard, loop-invariant, computed once per fragment by the caller
float computeShadow(int li, vec3 lPos, vec3 Ng, vec3 L, float NdotLg, float uvScale) {
    vec3 d = (fragPos + Ng * SHADOW_NORMAL_OFFSET * (1.0 - NdotLg)) - lPos;
    float bias = SHADOW_BIAS * (2.0 - NdotLg);

    vec3 a = abs(d);
    int face;
    float sc;
    float tc;
    float ma;
    if (a.x >= a.y && a.x >= a.z) {
        ma = a.x;
        if (d.x > 0.0) { face = 0; sc = -d.z; tc = -d.y; }
        else           { face = 1; sc =  d.z; tc = -d.y; }
    } else if (a.y >= a.z) {
        ma = a.y;
        if (d.y > 0.0) { face = 2; sc = d.x; tc =  d.z; }
        else           { face = 3; sc = d.x; tc = -d.z; }
    } else {
        ma = a.z;
        if (d.z > 0.0) { face = 4; sc =  d.x; tc = -d.y; }
        else           { face = 5; sc = -d.x; tc = -d.y; }
    }

    float zf = ma - bias;
    if (zf >= SHADOW_FAR || zf <= SHADOW_NEAR) return 1.0;
    float ref = SHADOW_A + SHADOW_B / zf;

    vec2 uv = uvScale * vec2(sc, tc) / ma + 0.5;
    return texture(shadowMaps, vec4(uv, float(li * 6 + face), ref));
}

// ------------------------------------------------------------
//  BRDFs. Everything that does not depend on L is precomputed once per fragment
// ------------------------------------------------------------
struct BrdfConst {
    // Cook-Torrance
    float a2;        // alpha^2 of GGX
    float kG;        // Schlick-GGX k
    float Gv;        // geometry term for the view direction (loop-invariant)
    float specScale; // dielectric/metal specular scale
    // Blinn-Phong
    float shininess;
    float normFactor;
};

const float DIELECTRIC_SPEC_SCALE = 0.5;

BrdfConst precomputeBrdf(float metallic, float roughness, float NdotV) {
    BrdfConst c;
    float alpha = roughness * roughness;
    c.a2 = alpha * alpha;
    float k = roughness + 1.0;
    c.kG = (k * k) * 0.125;
    c.Gv = NdotV / (NdotV * (1.0 - c.kG) + c.kG);
    c.specScale = mix(DIELECTRIC_SPEC_SCALE, 1.0, metallic);

    float invR = 1.0 - roughness;
    c.shininess = mix(2.0, 128.0, invR * invR);
    c.normFactor = (c.shininess + 2.0) / (8.0 * PI);
    return c;
}

// Normalized Blinn-Phong
vec3 evalFastBRDF(vec3 albedo, float metallic, vec3 F0, vec3 N, vec3 V, vec3 L,
                  float NdotL, BrdfConst c) {
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    vec3 F = F0 + (1.0 - F0) * pow5(1.0 - VdotH);

    vec3 f_specular = F * (c.normFactor * pow(NdotH, c.shininess)) * NdotL * PI;
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    return kD * albedo * NdotL + f_specular;
}

vec3 cookTorrance(vec3 albedo, float metallic, vec3 F0, vec3 N, vec3 V, vec3 L,
                  float NdotL, float NdotV, BrdfConst c) {
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);

    vec3 F = F0 + (1.0 - F0) * pow5(clamp(1.0 - max(dot(V, H), 0.0), 0.0, 1.0));

    float denom = max(NdotH * NdotH * (c.a2 - 1.0) + 1.0, 0.0001);
    float D = c.a2 / (PI * denom * denom);
    float G = c.Gv * (NdotL / (NdotL * (1.0 - c.kG) + c.kG));

    vec3 f_specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.0001) * NdotL * PI * c.specScale;
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    return kD * albedo * NdotL + f_specular;
}

vec3 computeLight(vec3 albedo) {
    float metallic = texture(textureMap[2], fragUV).r;
    float roughness = clamp(texture(textureMap[3], fragUV).r, 0.04, 1.0);
    float ao = texture(textureMap[4], fragUV).r;

    mat3 TBN = computeTBN(fragNorm, fragTan.xyz, fragTan.w);
    vec3 N = normalize(computeNormalMap(TBN));

    vec3 V = normalize(gubo.playerPos.xyz - fragPos);
    vec3 Ng = normalize(fragNorm);
    float NdotV = max(dot(N, V), 0.0001);

    vec3 F0 = computeF0(albedo, metallic);

    // ---- loop-invariant stuff, hoisted out of the light loop ----
    const int  lightCount = min(int(gubo.settings.z), NUM_LIGHTS);
    const bool useShadows = gubo.settings.x > 0.0;
    const int  mode       = int(gubo.settings.y);
    BrdfConst brdf = precomputeBrdf(metallic, roughness, NdotV);
    float shadowRes = float(textureSize(shadowMaps, 0).x);
    float uvScale = 0.5 * (1.0 - 2.0 * SHADOW_GUARD_TEXELS / shadowRes);

    vec3 direct = vec3(0.0);
    for (int i = 0; i < lightCount; i++) {
        vec4 lp = gubo.lightParams[i];
        if (lp.x < 1.0) continue; // light is off

        // 1) cheapest test first: squared distance vs. precomputed influence radius.
        //    No sqrt, no pow, no normalize for lights that are out of range.
        vec3 lPos = gubo.lightPos[i].xyz;
        vec3 toLight = lPos - fragPos;
        float d2 = dot(toLight, toLight);
        if (lp.z > 0.0 && d2 > lp.z) continue;

        float invD = inversesqrt(max(d2, 1e-8));
        vec3 L = toLight * invD;

        // 2) back-facing w.r.t. the shading normal: BRDF is zero
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        // 3) attenuation (only for surviving lights); pow(g/d, beta)
        vec3 lightCol = gubo.lightColor[i].rgb * pow(lp.y * invD, 2.0f);
        if (max(lightCol.r, max(lightCol.g, lightCol.b)) < LIGHT_CUTOFF) continue;

        // 4) shadow lookup: the most expensive step, so it comes last
        float visibility = 1.0;
        int torch = int(gubo.lightParams[i].w); // shadow maps are indexed by torch, not by slot
        float NdotLg = clamp(dot(Ng, L), 0.0, 1.0);
        if (gubo.settings.x > 0.0 && torch < textureSize(shadowMaps, 0).z / 6) {
            visibility = computeShadow(i, lPos, Ng, L, NdotLg, uvScale);
            if (visibility <= 0.0) continue;
        }

        vec3 fr;
        if (mode == 1)      fr = evalFastBRDF(albedo, metallic, F0, N, V, L, NdotL, brdf);
        else if (mode == 2) fr = cookTorrance(albedo, metallic, F0, N, V, L, NdotL, NdotV, brdf);
        else                fr = albedo * NdotL;

        direct += lightCol * fr * visibility;
    }

    vec3 ambColor = vec3(1.0, 0.3, 0.2);
    float specStrength = mix(1.0, 0.15, roughness);
    vec3 envSpecular = ambColor * F0 * specStrength * ao;
    vec3 envDiffuse = vec3(0.03) * ao * albedo * (1.0 - metallic);

    return direct + envDiffuse + envSpecular;
}

// ============================================================
//  MAIN
// ============================================================
void main() {
    vec4 base = texture(textureMap[0], fragUV); // sampled once instead of twice
    outColor = vec4(computeLight(base.rgb), base.a);
}
