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
	mat4 modelMatrix;
	float lightData; 
} push;


layout (location = 0) flat out int lightIndex;

void main() {
	int lightIndex = int(push.lightData);

	gl_Position = camera.projectionMatrix * camera.viewMatrix * push.modelMatrix * vVertex;
}
