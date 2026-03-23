#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout(push_constant) uniform BloomParams {
    float threshold;
    float strength;
} params;

layout(location = 0) out vec2 uvCoord;
layout(location = 1) flat out float threshold;
layout(location = 2) flat out float strength;

void main() {
    uvCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
    threshold = params.threshold;
    strength = params.strength;
}