#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout (location = 0) in  vec4 vVertex;
layout (location = 1) in  vec4 vNormal;
layout (location = 2) in  vec2 texCoords;

layout(set = 0 , binding = 0) uniform CameraUBO {
    mat4 projectionMatrix;
	mat4 viewMatrix;
} camera;

layout(push_constant) uniform Push {
	vec4 BloomTint;
	float BloomFactor;
} push;

layout (location = 1) out vec3 fragTexCoords;
layout (location = 2) flat out vec3 tint;
layout (location = 3) flat out float factor;


void main() {

	fragTexCoords = vVertex.xyz;
	mat4 view =  mat4(mat3(camera.viewMatrix));
	tint = push.BloomTint.rgb;
	factor = push.BloomFactor;

	gl_Position =  camera.projectionMatrix * view  * vVertex; 
}
