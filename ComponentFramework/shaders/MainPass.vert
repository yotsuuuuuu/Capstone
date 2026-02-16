#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout (location = 0) in  vec4 vVertex;
layout (location = 1) in  vec4 vNormal;
layout (location = 2) in  vec2 texCoords;

layout(set = 0 , binding = 0) uniform CameraUBO {
    mat4 projectionMatrix;
	mat4 viewMatrix;
} camera;

layout(set = 0, binding = 1) uniform GLightData{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 dir;
} GLData;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix; 
} push;

layout (location = 0) out vec3 vertNormal;
layout (location = 1) out vec3 eyeDir;
layout (location = 2) out vec2 fragTexCoords;
layout (location = 3) out vec3 lightDir;
layout (location = 4) out vec4 fragLightSpace;


void main() {
	fragTexCoords = texCoords;
	// Transform normals to view space
	mat3 normalMatrix = mat3(transpose(inverse(camera.viewMatrix * push.modelMatrix)));
	vertNormal = normalize(normalMatrix * vNormal.xyz);
	// Position in view space
	vec4 viewPos = camera.viewMatrix * push.modelMatrix * vVertex;
	vec3 viewVertPos = vec3(viewPos);
	// Eye direction 
	eyeDir = normalize(-viewVertPos);

	// If we are Trasnforming the normal to view space the light dir must match 
	// it is form pespective form the vertex so it must be negated
	lightDir = -normalize(mat3(camera.viewMatrix) * GLData.dir.xyz);

	// Fragment postion in light space
	fragLightSpace = GLData.projectionMatrix * GLData.viewMatrix * push.modelMatrix * vVertex;

	// Final Pos
	// viewpos  =  camera.viewMatrix * push.modelMatrix * vVertex;
	gl_Position = camera.projectionMatrix * viewPos;
}
