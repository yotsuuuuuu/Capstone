#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout (location = 0) in  vec4 vVertex;
layout (location = 1) in  vec4 vNormal;
layout (location = 2) in  vec2 texCoords;

layout(set = 0 , binding = 0) uniform CameraUBO {
    mat4 projectionMatrix;
	mat4 viewMatrix;
} camera;



layout (location = 1) out vec3 fragTexCoords;



void main() {

	fragTexCoords = vVertex.xyz;
	mat4 view =  camera.viewMatrix;
	view[3][0] = 0.0f;
	view[3][1] = 0.0f; 
	view[3][2] = 0.0f; 

	gl_Position =  camera.projectionMatrix * view  * vVertex; 
}
