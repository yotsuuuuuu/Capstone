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
	vec4 pos;
	vec4 pad;
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
	mat3 normalMatrix = mat3(push.normalMatrix);

	vertNormal = normalize(normalMatrix * vNormal.xyz); /// Rotate the normal to the correct orientation 
	vec3 vertPos = vec3(camera.viewMatrix * push.modelMatrix * vVertex); /// This is the position of the vertex from the origin
	vec3 vertDir = normalize(vertPos);
	eyeDir = -vertDir;

	/// Light position from the point-of-view of each vertex
	vec3 lightLocFromVertex =  vec3(GLData.pos) - vertPos; 
	lightDir = normalize(lightLocFromVertex); 
	
	fragLightSpace = GLData.projectionMatrix * GLData.viewMatrix * push.modelMatrix * vVertex;
	
	gl_Position =  camera.projectionMatrix * camera.viewMatrix * push.modelMatrix * vVertex; 
}
