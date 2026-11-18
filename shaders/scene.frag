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
    vec4 lightPos[NUM_LIGHTS]; // xyz = world position of light i
    vec4 lightParams[NUM_LIGHTS]; // x = on/off, y = g, z = beta
    vec4 playerPos;
    vec4 extra;
} gubo;

// ------------------------------------------------------------
//  TEXTURE MAPS
// ------------------------------------------------------------
layout(binding = 2, set = 0) uniform sampler2D textureMap[5];

// Omnidirectional shadow maps of all the lights: depth, 6 layers per light (+X -X +Y -Y +Z -Z).
// Hardware depth comparison + bilinear filtering (sampler with compareEnable).
layout(binding = 3, set = 0) uniform sampler2DArrayShadow shadowMaps;

const float PI = 3.14159265359;

vec3 pointLightColor(int light_i, vec3 fragPosition, out vec3 L) {
    vec3 toLight = gubo.lightPos[light_i].xyz - fragPosition;
    float dist = max(length(toLight), 0.0001);
    L = toLight / dist;

    float g = gubo.lightParams[light_i].y;
    float ratio = g / dist;
    //float beta = gubo.lightParams[light_i].z;

    float decay = ratio * ratio;
    //float decay = pow(g / dist, beta);
    return gubo.lightColor[light_i].rgb * decay;
}

mat3 computeTBN(vec3 N, vec3 T, float tangentW) {
    N = normalize(N);
    T = normalize(T - dot(T, N) * N); // Gram-Schmidt re-orthogonalization
    vec3 B = cross(N, T) * tangentW; // w holds the handedness (+1 / -1)
    return mat3(T, B, N); // columns: tangent space -> world space
}

vec3 computeNormalMap(mat3 TBN) {
    vec3 n = texture(textureMap[1], fragUV).rgb * 2.0 - 1.0;
    return normalize(TBN * n); // tangent space -> world space
}

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float dotVH, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - dotVH, 0.0, 1.0), 5.0);
}

// GGX Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    // FIX: roughness is perceptual (the geometry term below assumes that), so alpha = r^2 and a2 = alpha^2.
    // Before, a2 = r^2, which made the lobe far too wide and dim -> metals looked dull/dark.
    float alpha = roughness * roughness;
    float a2 = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
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

// ------------------------------------------------------------
//  OMNIDIRECTIONAL SHADOWS (hardware PCF)
// ------------------------------------------------------------
// These three MUST match the C++ side
#define SHADOW_NEAR          0.1    // = ShadowMapperParams::nearPlane
#define SHADOW_FAR           100.0  // = ShadowMapperParams::farPlane
#define SHADOW_GUARD_TEXELS  4.0    // = ShadowMapper::GuardTexels

// tune to your scene scale
const float SHADOW_BIAS = 0.05; // world units along the face axis
const float SHADOW_NORMAL_OFFSET = 0.05; // world units, applied at grazing angles
const float SHADOW_PCF_TEXELS = 1.5; // tap offset; each tap is already a bilinear 2x2 comparison
const float LIGHT_CUTOFF = 1e-4; // lights contributing less than this are skipped

const float SHADOW_A = SHADOW_FAR / (SHADOW_FAR - SHADOW_NEAR); // stored depth = A + B / zf
const float SHADOW_B = -SHADOW_FAR * SHADOW_NEAR / (SHADOW_FAR - SHADOW_NEAR);

// returns 1.0 = fully lit by light li, 0.0 = fully in shadow
float computeShadow(int li, vec3 lPos, vec3 Ng, vec3 L) {
    float NdotL = clamp(dot(Ng, L), 0.0, 1.0);
    vec3 d = (fragPos + Ng * SHADOW_NORMAL_OFFSET * (1.0 - NdotL)) - lPos;
    float bias = SHADOW_BIAS * (1.0 + (1.0 - NdotL));

    // cube face selection (Vulkan spec), done ONCE per light
    vec3 a = abs(d);
    int face;
    float sc;
    float tc;
    float ma;
    if (a.x >= a.y && a.x >= a.z) {
        ma = a.x;
        if (d.x > 0.0) {
            face = 0;
            sc = -d.z;
            tc = -d.y;
        } else {
            face = 1;
            sc = d.z;
            tc = -d.y;
        }
    } else if (a.y >= a.z) {
        ma = a.y;
        if (d.y > 0.0) {
            face = 2;
            sc = d.x;
            tc = d.z;
        } else {
            face = 3;
            sc = d.x;
            tc = -d.z;
        }
    } else {
        ma = a.z;
        if (d.z > 0.0) {
            face = 4;
            sc = d.x;
            tc = -d.y;
        } else {
            face = 5;
            sc = -d.x;
            tc = -d.y;
        }
    }

    // reference depth of this fragment (moved towards the light by the bias)
    float zf = ma - bias;
    if (zf >= SHADOW_FAR || zf <= SHADOW_NEAR) return 1.0; // outside the shadow range: lit
    float ref = SHADOW_A + SHADOW_B / zf;

    // texture coordinates on the face (the face is rendered with a guard band, see ShadowMapper)
    float res = float(textureSize(shadowMaps, 0).x);
    float guard = 1.0 - 2.0 * SHADOW_GUARD_TEXELS / res;
    vec2 uv = 0.5 * guard * vec2(sc, tc) / ma + 0.5;
    float layer = float(li * 6 + face);
    float o = SHADOW_PCF_TEXELS / res;

    // 4 taps, each a hardware bilinear depth comparison = 16 comparisons
    float lit = texture(shadowMaps, vec4(uv + vec2(-o, -o), layer, ref))
            + texture(shadowMaps, vec4(uv + vec2(o, -o), layer, ref))
            + texture(shadowMaps, vec4(uv + vec2(-o, o), layer, ref))
            + texture(shadowMaps, vec4(uv + vec2(o, o), layer, ref));
    return lit * 0.25;
}

vec3 evalDiffuseOnlyLight(
    vec3 albedo,
    float ao,
    vec3 fragPosition,
    vec3 normalWorld,
    vec4 tangentWorld,
    vec2 uv
) {
    // 1. Decode normal map into world space
    mat3 TBN = computeTBN(normalWorld, tangentWorld.xyz, tangentWorld.w);
    vec3 N = normalize(computeNormalMap(TBN));

    // 2. Accumulate diffuse contributions across active lights
    vec3 direct = vec3(0.0);
    for (int i = 0; i < NUM_LIGHTS; i++) {
        if (gubo.lightParams[i].x < 1.0) continue;

        vec3 L;
        vec3 lightCol = pointLightColor(i, fragPosition, L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0 || max(lightCol.r, max(lightCol.g, lightCol.b)) < LIGHT_CUTOFF) continue;

        direct += lightCol * albedo * NdotL;
    }

    // 3. Add ambient floor modulated by AO
    vec3 ambient = vec3(0.03) * ao * albedo;
    return direct + ambient;
}

// Normalized Blinn-Phong
vec3 evalFastBRDF(
    vec3 albedo,
    float metallic,
    float roughness,
    vec3 F0,
    vec3 N,
    vec3 V,
    vec3 L,
    float NdotL,
    float NdotV
) {
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // Cheap Schlick Fresnel (no pow)
    float f = 1.0 - VdotH;
    float f2 = f * f;
    vec3 F = F0 + (1.0 - F0) * (f2 * f2 * f);

    // Roughness [0..1] -> shininess [128..2] so matte surfaces spread out
    float invR = 1.0 - roughness;
    float shininess = mix(2.0, 128.0, invR * invR);

    // Normalized Blinn-Phong lobe
    float normFactor = (shininess + 2.0) / (8.0 * PI);
    vec3 f_specular = F * (normFactor * pow(NdotH, shininess)) * NdotL * PI; // FIX: *PI, diffuse has no 1/PI

    // Energy-conserving diffuse (matches your original brightness scale)
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 f_diffuse = kD * albedo * NdotL;

    return f_diffuse + f_specular;
}

// Specular strength for non-metals in the Cook-Torrance path (1.0 = physically scaled, lower = less "wet").
// Metals are not affected (they always use 1.0), so they keep their full highlight.
const float DIELECTRIC_SPEC_SCALE = 0.5;

vec3 cookTorrance(
    vec3 albedo,
    float metallic,
    float roughness,
    vec3 F0,
    vec3 N,
    vec3 V,
    vec3 L,
    float NdotL,
    float NdotV
) {
    vec3 H = normalize(V + L);
    // ---- Cook-Torrance terms ----------------
    vec3 F = fresnelSchlick(max(dot(V, H), 0.0), F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 f_diffuse = albedo * NdotL;
    // FIX: guard the denominator (NdotV can be 0 -> inf/NaN) and multiply by NdotL,
    // like the diffuse term and like evalFastBRDF do (fr already includes cos(theta_i)).
    vec3 f_specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.0001) * NdotL * PI * mix(DIELECTRIC_SPEC_SCALE, 1.0, metallic);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic); // energy conservation; metals have no diffuse
    return kD * f_diffuse + f_specular;
}

vec3 computeLight() {
    vec3 albedo = texture(textureMap[0], fragUV).rgb;
    float metallic = texture(textureMap[2], fragUV).r;
    float roughness = texture(textureMap[3], fragUV).r;
    float ao = texture(textureMap[4], fragUV).r;

    // ---- Build TBN and decode the normal map ------
    mat3 TBN = computeTBN(fragNorm, fragTan.xyz, fragTan.w);
    vec3 N = normalize(computeNormalMap(TBN));

    // ---- Geometry setup ---------------------
    vec3 V = normalize(gubo.playerPos.xyz - fragPos);
    vec3 Ng = normalize(fragNorm); // geometric normal, used for shadows
    float NdotV = max(dot(N, V), 0.0001);

    vec3 F0 = computeF0(albedo, metallic);

    // ---- Sum of the contributions of all the lights ------------
    vec3 direct = vec3(0.0);
    for (int i = 0; i < NUM_LIGHTS; i++) {
        if (gubo.lightParams[i].x < 1.0) continue; //Light is off

        vec3 lPos = gubo.lightPos[i].xyz;
        vec3 L;
        vec3 lightCol = pointLightColor(i, fragPos, L); // fills L

        float NdotL = max(dot(N, L), 0.0);

        // early-outs: skip all the BRDF + shadow work when the light cannot contribute
        if (NdotL <= 0.0) continue; // fr == 0 (G term vanishes)
        if (max(lightCol.r, max(lightCol.g, lightCol.b)) < LIGHT_CUTOFF) continue; // too far / too dim

        float visibility = 1.0;
        if (gubo.extra.x > 0.0) {
            visibility = computeShadow(i, lPos, Ng, L);
        }

        if (visibility <= 0.0) continue;

        vec3 fr;
        switch (int(gubo.extra.y)) {
            case 1:
            {
                fr = cookTorrance(albedo, metallic, roughness, F0, N, V, L, NdotL, NdotV);
            }
            break;
            case 2:
            {
                fr = evalFastBRDF(albedo, metallic, roughness, F0, N, V, L, NdotL, NdotV);
            }
            break;
            default:
            {
                fr = albedo * NdotL;
            }
            break;
        }

        direct += lightCol * fr * visibility;
    }

    vec3 ambient = vec3(0.03) * ao * albedo;
    vec3 color = direct + ambient;
    color = color / (color + vec3(1.0));
    return color;
}

// ============================================================
//  MAIN
// ============================================================
void main() {
    outColor = vec4(computeLight(), 1.0);
}
