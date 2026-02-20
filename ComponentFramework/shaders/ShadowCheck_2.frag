#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_SHADOW_MAPS 3

layout(location = 0) in vec3 vertNormal;
layout(location = 1) in vec3 eyeDir;
layout(location = 2) in vec2 fragTexCoords;
layout(location = 3) in vec3 lightDir;
layout(location = 4) in vec4 fragLightSpace[MAX_SHADOW_MAPS];

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 2) uniform sampler2DShadow shadowMap[MAX_SHADOW_MAPS];

// Debug helper function
vec3 DebugLightSpace(vec4 lightPos, int cascadeIndex) {
    vec3 ndc = lightPos.xyz / lightPos.w;
    vec2 uv = ndc.xy * 0.5 + 0.5;
    float depth = ndc.z;

    // Highlight invalid UVs and Depth
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 
        || depth < 0.0 || depth > 1.0) {
        return vec3(0.6, 0.0, 0.6); // magenta for invalid
    }


    // Show shadow test 
    float shadow = texture(shadowMap[cascadeIndex], vec3(uv, depth));
    if (shadow < 0.5) {
        // Not in light
        return vec3(0.2, 0.2, 0.2); // dark grey = shadowed
    } else {
        // Lit by light
        return vec3(uv, 1.0 - depth); // encode uv/depth in color for visualization
    }
}

void main() {
    vec3 color = vec3(0.0);

    // Combine multiple cascades (overlay or choose one)
    // Example: show highest-resolution cascade first
    for (int i = 0; i < MAX_SHADOW_MAPS; i++) {
        color = DebugLightSpace(fragLightSpace[i], i);
        // Uncomment below to mix cascades with additive debug colors
        //color += DebugLightSpace(fragLightSpace[i], i) / float(MAX_SHADOW_MAPS);
    }

    fragColor = vec4(color, 1.0);
}