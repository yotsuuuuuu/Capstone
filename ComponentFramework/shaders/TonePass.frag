#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 uvcoord;


layout( set = 0, binding = 0 ) uniform sampler2D hdrSampler;
//Just for testing if HDR frame are working.
void main() { 
	fragColor = texture(hdrSampler,uvcoord);
} 

