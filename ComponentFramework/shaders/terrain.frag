#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

// ALL bindings are in Set 0 now
layout(binding = 1) uniform LightsData {  // Set 0, Binding 1
    vec4 pos[4];
    vec4 diffuse[4];
    vec4 specular[4];
    vec4 ambient;
    uint numLights;
} lights;

layout(binding = 2) uniform sampler2D texSampler;  // Set 0, Binding 2

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 color = texture(texSampler, fragTexCoord).rgb;
    
    // Simple lighting (using the UBO from binding 1)
    vec3 lightDir = normalize(lights.pos[0].xyz);
    float diff = max(dot(normal, lightDir), 0.2);
    
    outColor = vec4(color * diff, 1.0);
}