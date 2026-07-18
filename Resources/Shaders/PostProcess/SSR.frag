#version 450 core
layout (location = 0) out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_ColorTex;
layout(binding = 1) uniform sampler2D u_NormalTex;
layout(binding = 2) uniform sampler2D u_DepthTex;

uniform mat4 MATRIX_PROJECTION;
uniform mat4 MATRIX_INVERSE_PROJECTION;
uniform mat4 MATRIX_VIEW;

uniform float NEAR_PLANE = 0.1;
uniform float FAR_PLANE = 500.0;

const float THICKNESS = 0.1;
const float MARCH_STEP = 0.1;
const int MARCH_MAX_STEPS = 100;
const int BINARY_SEARCH_STEPS = 10;

vec3 ReconstructViewPos(vec2 uv, float depth) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = MATRIX_INVERSE_PROJECTION * ndc;
    return viewPos.xyz / viewPos.w; // Perspective
}

vec2 RayToScreen(vec3 rayPos){
    vec4 projected = MATRIX_PROJECTION * vec4(rayPos, 1.0);
    projected.xyz /= projected.w; // NDC [-1, 1] space
    vec2 screenUV = projected.xy * 0.5 + 0.5;
    return screenUV;
}

vec3 BinarySearch(vec3 rayPos, vec3 reflectDir){
    vec3 refinePos = rayPos;
    float refineStep = MARCH_STEP * 0.5;
    refinePos -= reflectDir * refineStep; // Half step back
    for (int j = 0; j < BINARY_SEARCH_STEPS; j++) {
        vec4 refProj = MATRIX_PROJECTION * vec4(refinePos, 1.0);
        refProj.xyz /= refProj.w;
        vec2 refUV = refProj.xy * 0.5 + 0.5;

        float refDepth = texture(u_DepthTex, refUV).r;
        vec3 refGeom = ReconstructViewPos(refUV, refDepth);
        float refDiff = refinePos.z - refGeom.z;

        if (refDiff < 0.0) {
            refinePos -= reflectDir * refineStep; // Still inside the object, step backward again
        } else {
            refinePos += reflectDir * refineStep; // Stepped out, so step forward
        }
        refineStep *= 0.5; // Next iter. with smaller step
    }
    return refinePos;
}

float RandomNoise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec4 normalSample = texture(u_NormalTex, v_uv);
    vec3 baseColor = texture(u_ColorTex, v_uv).rgb;
    float power = normalSample.w;
    if(power < 0.1){
        FragColor = vec4(baseColor, 1.0);
        return;
    }
    float depth = texture(u_DepthTex, v_uv).r;
    // Skybox check
    if (depth >= 0.99) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    vec3 viewPos = ReconstructViewPos(v_uv, depth);
    vec3 viewDir = normalize(viewPos);
    vec3 worldNormal = texture(u_NormalTex, v_uv).xyz;
    vec3 viewNormal = normalize(mat3(MATRIX_VIEW) * worldNormal);
    vec3 reflectDir = normalize(reflect(viewDir, viewNormal));

    // Add start bias
    const float bias = 2;
    vec3 rayPos = viewPos + (viewNormal * MARCH_STEP * bias); // val 2.0 ??
    float jitter = RandomNoise(gl_FragCoord.xy);
    rayPos += reflectDir * MARCH_STEP * jitter;

    vec3 reflectionColor = vec3(0.0);
    float reflectionMask = 0.0;

    // Raymarching
    for(int i = 0; i < MARCH_MAX_STEPS; i++) {
        rayPos += reflectDir * MARCH_STEP;
        vec2 screenUV = RayToScreen(rayPos);
        // safety offscreen
        if(screenUV.x < 0.0 || screenUV.x > 1.0 || screenUV.y < 0.0 || screenUV.y > 1.0) {
            break;
        }
        // safety deadzone
        if (distance(screenUV, v_uv) < 0.01) {
            continue;
        }
        float sampleDepth = texture(u_DepthTex, screenUV).r;
        if (sampleDepth >= 0.9999) {
            continue;
        }
        vec3 geometryPos = ReconstructViewPos(screenUV, sampleDepth);
        float depthDiff = rayPos.z - geometryPos.z;
        // potential surface hit
        if (depthDiff < 0.0 && depthDiff > -THICKNESS) {
            vec3 refinePos = BinarySearch(rayPos, reflectDir);
            // Recalculate the final UV after honing in on the exact spot
            vec4 finalProj = MATRIX_PROJECTION * vec4(refinePos, 1.0);
            finalProj.xyz /= finalProj.w;
            screenUV = finalProj.xy * 0.5 + 0.5;

            // Backface Rejection
            vec3 hitWorldNormal = texture(u_NormalTex, screenUV).xyz;
            vec3 hitViewNormal = normalize(mat3(MATRIX_VIEW) * hitWorldNormal);
            if (dot(reflectDir, hitViewNormal) > 0.0) {
                continue;
            }
            reflectionColor = texture(u_ColorTex, screenUV).rgb;
            // Edge Fading
            const float b = 0.05;
            vec2 edgeFade = smoothstep(0.0, b, screenUV) * (1.0 - smoothstep(1.0 - b, 1.0, screenUV));
            reflectionMask = edgeFade.x * edgeFade.y;
            break;
        }
    }

    vec3 finalColor = baseColor + (reflectionColor * reflectionMask * 0.11);
    FragColor = vec4(finalColor, 1.0);
}