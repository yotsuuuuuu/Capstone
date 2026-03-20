#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL



layout (location = 0) in  vec4 vVertex;
layout (location = 1) in  vec4 vNormal;
layout (location = 2) in  vec2 texCoords;

layout(set = 0 , binding = 0) uniform CameraUBO {
    mat4 projectionMatrix;
	mat4 viewMatrix;
} camera;

//layout(push_constant) uniform Push {
//	mat4 modelMatrix;
//	float lightData; 
//} push;


struct PointLight
{
    vec4 position_radius;
    vec4 color_intensity;
    vec4 direction_inner;
    vec4 outer_type_pad;
};

layout(std430, set = 0 , binding = 5) readonly restrict buffer lightSSBO
{
    PointLight pointLight[];
};



layout (location = 0) flat out int lightIndex;

void main() {
	lightIndex = gl_InstanceIndex;
    PointLight light = pointLight[gl_InstanceIndex];
    vec3 worldPos = light.position_radius.xyz;
    float radius = 1.0;

    vec4 scaledVertex = vec4(vVertex.xyz * radius, 1.0);
    vec4 worldVertex = scaledVertex + vec4(worldPos, 0.0);

	gl_Position = camera.projectionMatrix * camera.viewMatrix * worldVertex;
}
