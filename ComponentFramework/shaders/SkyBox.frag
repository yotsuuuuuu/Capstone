#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL


layout (location = 1) in vec3 fragTexCoords;
layout (location = 2) flat in vec3 tint;
layout (location = 3) flat in float factor;

layout (location = 0) out vec4 fragColor;



layout(set = 1, binding = 0) uniform samplerCube skyBoxSampler;


void main() { 
	vec3 skyColor = texture(skyBoxSampler, fragTexCoords).rgb;
    
    float skyLuminance = dot(skyColor, vec3(0.2126, 0.7152, 0.0722));
    
    vec3 bloomTint = tint; 
    float bloomBoost = factor;
    
    vec3 result = skyColor + (skyLuminance * bloomBoost * bloomTint);
    fragColor = vec4(result, 1.0);
	//fragColor =  vec4(0.3,0,0,1);
} 

