#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL




layout (location = 0) flat in int lightIndex;


layout (location = 0) out vec4 fragColor;

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



layout(set = 1, binding = 0) uniform sampler2D texSampler;


void main() { 
	PointLight light = pointLight[lightIndex];
    float bloomfactor = 5.0;
    vec3 color = light.color_intensity.xyz ;
    float intensity = light.color_intensity.w * bloomfactor;
    fragColor = vec4(color * intensity, 1.0);	
} 

