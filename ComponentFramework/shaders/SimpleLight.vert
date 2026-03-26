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

mat4 buildModelMatrix(vec3 position, vec3 direction);

void main() {
	lightIndex = gl_InstanceIndex;
//    PointLight light = pointLight[gl_InstanceIndex];
//    vec3 worldPos = light.position_radius.xyz;
//    float radius = 1.0;
//
//    vec4 scaledVertex = vec4(vVertex.xyz * radius, 1.0);
//    vec4 worldVertex = scaledVertex + vec4(worldPos, 0.0);
//
//	gl_Position = camera.projectionMatrix * camera.viewMatrix * worldVertex;

    PointLight light = pointLight[gl_InstanceIndex];
    vec3 worldPos = light.position_radius.xyz;
    vec3 direction = light.direction_inner.xyz;

    mat4 model = buildModelMatrix(worldPos, direction);
    gl_Position = camera.projectionMatrix * camera.viewMatrix * model * vVertex;
}


mat4 buildModelMatrix(vec3 position, vec3 direction)
{
    vec3 forward = normalize(direction);
    vec3 up = abs(forward.y) < 0.999 ? vec3(0,1,0) : vec3(1,0,0); // avoid gimbal
    vec3 right = normalize(cross(up, forward));
    up = cross(forward, right);

    mat4 model = mat4(1.0);
    model[0] = vec4(right,   0.0);
    model[1] = vec4(up,      0.0);
    model[2] = vec4(-forward, 0.0);
    model[3] = vec4(position, 1.0);
    return model;
}