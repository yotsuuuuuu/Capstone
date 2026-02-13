#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL


layout (location = 1) in vec3 fragTexCoords;

layout (location = 0) out vec4 fragColor;



layout(set = 1, binding = 0) uniform samplerCube skyBoxSampler;


void main() { 
	
	fragColor = texture(skyBoxSampler,fragTexCoords);
	//fragColor =  vec4(0.3,0,0,1);
} 

