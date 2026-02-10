#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

#define MAX_LIGHTS 4

layout (location = 0) in  vec4 vVertex;
layout (location = 1) in  vec4 vNormal;
layout (location = 2) in  vec2 texCoords;

layout(set = 0 ,binding = 0) uniform LightsUBO {
	mat4 projectionMatrix;
	mat4 viewMatrix; 
	vec4 pad;
	vec4 pad1;
	vec4 pad2;
	vec4 pad3;
	vec4 pad4;
} light;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix; 
} push;


void main() {

	gl_Position =  light.projectionMatrix * light.viewMatrix * push.modelMatrix * vVertex; 
}
