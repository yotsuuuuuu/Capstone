#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 fragTexCoords;
layout (location = 3) in vec3 lightDir;
layout (location = 4) in vec4 fragLightSpace;

layout (location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform GLightData{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 dir;
} GLData;

layout(set = 0, binding = 2) uniform sampler2DShadow shadowMap;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

vec3 ShadowCheck(vec4 pos) {

	// going form clip space to NDC space
	vec3 ndc = pos.xyz/ pos.w;
	// Vulkan ndc  -1 to 1 for x and y and z form 0 - 1
	// so need to remap to texture jsut x and y
	vec2 uv =  ndc.xy * 0.5 + 0.5;
	float depth = ndc.z;

	vec3 color = vec3(uv, depth);

	if (uv.x < 0 || uv.x > 1 ||
	    uv.y < 0 || uv.y > 1 
        )
	{
	 color = vec3(0.6,0,0.6); // bright magenta = invalid
	}
	
	if(depth < 0 || depth > 1 ) {
	
	color += vec3(0,0.5,0);
	}

	return color;
	
}


void main() { 
	fragColor = vec4(ShadowCheck(fragLightSpace),1);;
} 

