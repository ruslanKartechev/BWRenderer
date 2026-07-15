#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 v_uv;

uniform sampler2D u_ColorTex;
uniform sampler2D u_NormalTex;
uniform sampler2D u_DepthTex;

uniform mat4 MATRIX_PROJECTION;
uniform mat4 MATRIX_INVERSE_PROJECTION;
uniform mat4 MATRIX_VIEW;

uniform float NEAR_PLANE = 0.1;
uniform float FAR_PLANE = 500.0;

// --- RAYMARCHING PARAMETERS ---
const float MARCH_STEP = 0.1;
const float thickness = 0.2; // Failsafe so we don't reflect the back of an object behind the floor
const int MARCH_MAX_STEPS = 100;
const int binarySearchSteps = 10;

// Helper function: Converts 2D UV + Depth into a 3D View Space position
vec3 ReconstructViewPos(vec2 uv, float depth) {
    // Convert UV (0 to 1) to Normalized Device Coordinates (-1 to 1)
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = MATRIX_INVERSE_PROJECTION * ndc;
    return viewPos.xyz / viewPos.w; // Perspective divide
}

vec2 RayToScreen(vec3 rayPos){
    vec4 projected = MATRIX_PROJECTION * vec4(rayPos, 1.0);
    projected.xyz /= projected.w; // NDC [-1, 1] space
    vec2 screenUV = projected.xy * 0.5 + 0.5;
    return screenUV;
}

vec3 binarySearch(vec3 rayPos, vec3 reflectDir){
    vec3 refinePos = rayPos;
    float refineStep = MARCH_STEP * 0.5;
    refinePos -= reflectDir * refineStep; // Step back half a step to start
    // 5 iterations is usually the sweet spot for performance vs. quality
    for (int j = 0; j < binarySearchSteps; j++) {
        vec4 refProj = MATRIX_PROJECTION * vec4(refinePos, 1.0);
        refProj.xyz /= refProj.w;
        vec2 refUV = refProj.xy * 0.5 + 0.5;

        float refDepth = texture(u_DepthTex, refUV).r;
        vec3 refGeom = ReconstructViewPos(refUV, refDepth);
        float refDiff = refinePos.z - refGeom.z;

        refineStep *= 0.5; // Halve the step size for the next iteration
        if (refDiff < 0.0) {
            // Still inside the object, step backward again
            refinePos -= reflectDir * refineStep;
        } else {
            // Stepped out into the air, step forward
            refinePos += reflectDir * refineStep;
        }
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
//    FragColor = vec4(power);
//    return;

    if(power < 0.1){
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    float depth = texture(u_DepthTex, v_uv).r;
    // If depth is 1.0, it's the skybox. Don't reflect on the sky!
    if (depth >= 0.9999) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    vec3 viewPos = ReconstructViewPos(v_uv, depth);
    vec3 viewDir = normalize(viewPos);
    vec3 worldNormal = texture(u_NormalTex, v_uv).xyz;
    vec3 viewNormal = normalize(mat3(MATRIX_VIEW) * worldNormal);
    vec3 reflectDir = normalize(reflect(viewDir, viewNormal));

    // Add start bias
    const float bias = 1.5;
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
        if (distance(screenUV, v_uv) < 0.015) {
            continue;
        }

        float sampleDepth = texture(u_DepthTex, screenUV).r;
        if (sampleDepth >= 0.9999) {
            continue;
        }

        vec3 geometryPos = ReconstructViewPos(screenUV, sampleDepth);
        float depthDiff = rayPos.z - geometryPos.z;
        // potential surface hit
        if (depthDiff < 0.0 && depthDiff > -thickness) {
            vec3 refinePos = binarySearch(rayPos, reflectDir);
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
            vec2 edgeFade = smoothstep(0.0, 0.05, screenUV) * (1.0 - smoothstep(0.95, 1.0, screenUV));
            reflectionMask = edgeFade.x * edgeFade.y;
            break;
        }
    }

    vec3 finalColor = baseColor + (reflectionColor * reflectionMask * 0.5);
    FragColor = vec4(finalColor, 1.0);
}