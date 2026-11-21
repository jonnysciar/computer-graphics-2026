#version 450
// Bake pass: for every lightmap texel, accumulate the STATIC lights (indices [settings.w, settings.z)).
// Uses the same attenuation and the same shadow maps as the runtime shader, so baked == dynamic look.
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;

layout(location = 0) out vec4 outIrradiance; // rgb = irradiance, a = 1 (coverage mask for dilation)
layout(location = 1) out vec4 outDirection;  // xyz = luminance-weighted sum of directions

#define NUM_LIGHTS 19
layout(binding = 1, set = 0) uniform GlobalUniformBufferObject {
    vec4 lightColor[NUM_LIGHTS];
    vec4 lightPos[NUM_LIGHTS];
    vec4 lightParams[NUM_LIGHTS]; // x = on/off, y = g, z = beta, w = squared radius (0 = unbounded)
    vec4 playerPos;
    vec4 settings;                // z = total lights, w = number of dynamic lights
} gubo;
layout(binding = 3, set = 0) uniform sampler2DArrayShadow shadowMaps;

#define SHADOW_NEAR          0.1
#define SHADOW_FAR           100.0
#define SHADOW_GUARD_TEXELS  4.0
const float SHADOW_BIAS = 0.05;
const float SHADOW_NORMAL_OFFSET = 0.05;
const float SHADOW_A = SHADOW_FAR / (SHADOW_FAR - SHADOW_NEAR);
const float SHADOW_B = -SHADOW_FAR * SHADOW_NEAR / (SHADOW_FAR - SHADOW_NEAR);
const float LIGHT_CUTOFF = 1e-4;

// identical to the runtime version
float computeShadow(int li, vec3 lPos, vec3 Ng, float NdotLg, float uvScale) {
    vec3 d = (fragPos + Ng * SHADOW_NORMAL_OFFSET * (1.0 - NdotLg)) - lPos;
    float bias = SHADOW_BIAS * (2.0 - NdotLg);
    vec3 a = abs(d);
    int face; float sc; float tc; float ma;
    if (a.x >= a.y && a.x >= a.z) {
        ma = a.x;
        if (d.x > 0.0) { face = 0; sc = -d.z; tc = -d.y; } else { face = 1; sc = d.z; tc = -d.y; }
    } else if (a.y >= a.z) {
        ma = a.y;
        if (d.y > 0.0) { face = 2; sc = d.x; tc = d.z; } else { face = 3; sc = d.x; tc = -d.z; }
    } else {
        ma = a.z;
        if (d.z > 0.0) { face = 4; sc = d.x; tc = -d.y; } else { face = 5; sc = -d.x; tc = -d.y; }
    }
    float zf = ma - bias;
    if (zf >= SHADOW_FAR || zf <= SHADOW_NEAR) return 1.0;
    float ref = SHADOW_A + SHADOW_B / zf;
    vec2 uv = uvScale * vec2(sc, tc) / ma + 0.5;
    return texture(shadowMaps, vec4(uv, float(li * 6 + face), ref));
}

void main() {
    vec3 Ng = normalize(fragNorm);
    float res = float(textureSize(shadowMaps, 0).x);
    float uvScale = 0.5 * (1.0 - 2.0 * SHADOW_GUARD_TEXELS / res);

    vec3 E = vec3(0.0);
    vec3 dirSum = vec3(0.0);

    for (int i = int(gubo.settings.w); i < min(int(gubo.settings.z), NUM_LIGHTS); i++) {
        vec4 lp = gubo.lightParams[i];
        if (lp.x < 1.0) continue;

        vec3 toLight = gubo.lightPos[i].xyz - fragPos;
        float d2 = dot(toLight, toLight);
        if (lp.w > 0.0 && d2 > lp.w) continue;
        float invD = inversesqrt(max(d2, 1e-8));
        vec3 L = toLight * invD;

        float NdotL = max(dot(Ng, L), 0.0);
        if (NdotL <= 0.0) continue;

        vec3 c = gubo.lightColor[i].rgb * pow(lp.y * invD, lp.z);
        if (max(c.r, max(c.g, c.b)) < LIGHT_CUTOFF) continue;

        float vis = computeShadow(i, gubo.lightPos[i].xyz, Ng, NdotL, uvScale);
        vec3 contrib = c * NdotL * vis;
        E += contrib;
        dirSum += L * dot(contrib, vec3(0.2126, 0.7152, 0.0722));
    }

    outIrradiance = vec4(E, 1.0);
    outDirection = vec4(dirSum, 1.0);
}
