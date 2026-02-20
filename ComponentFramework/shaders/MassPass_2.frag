#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

#define MAX_SHADOW_MAPS 3

layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 fragTexCoords;
layout (location = 3) in vec3 lightDir;
layout (location = 4) in vec4 fragLightSpace;

layout (location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform GLightData{
	mat4 projectionMatrix[MAX_SHADOW_MAPS];
	mat4 viewMatrix[MAX_SHADOW_MAPS];
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 dir;
} GLData;


layout(set = 0, binding = 2) uniform sampler2DShadow shadowMap[MAX_SHADOW_MAPS];

layout(set = 1, binding = 0) uniform sampler2D texSampler;



float ShadowCheck(vec4 pos) {

	// going form clip space to NDC space
	float invW = 1.0 / pos.w;
	
	vec3 ndc = pos.xyz * invW;
	// Vulkan ndc  -1 to 1 for x and y and z form 0 - 1
	// so need to remap to texture jsut x and y
	vec2 uv =  ndc.xy * 0.5 + 0.5;

	float minBias = 0.0005;
	float slopeFactor = 0.0005;
	float NL = max( dot(normalize(vertNormal), normalize(lightDir)) ,0.0);
	float bias = max(minBias,slopeFactor * (1 - NL));
	float depth = ndc.z - bias;
	float clampedDepth = clamp(depth, 0.0, 1.0);
	vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

	int kernelHalf = 1;
	int totalSamples = 0;
	float result = 0.0;
	for(int x = -kernelHalf; x<= kernelHalf ;x++){
		for(int y = -kernelHalf; y<= kernelHalf ;y++){
			vec2 offset = vec2(x,y)* texelSize;
			result += texture(shadowMap, vec3(uv + offset, clampedDepth));
			totalSamples++;
		}
	}

	//return  texture(shadowMap,vec3(uv,clampedDepth));

    return result /float(totalSamples);

	
}


void main() { 
	vec3 reflection;
	float spec;
	float diff;
	vec4 ka = GLData.ambient;
	vec4 kd = GLData.diffuse;
	vec4 ks = GLData.specular;
	vec4 kt = texture(texSampler, fragTexCoords);
	
	// Ambient 
	vec4 phongResult = ka * kt;

	// 
	float shadow = ShadowCheck(fragLightSpace);
	//float shadow = 1.0; // Temporary - no shadows
	
	diff = max(dot(vertNormal, lightDir), 0);		
	reflection = normalize(reflect(-lightDir, vertNormal));
	spec = max(dot(eyeDir, reflection), 0.0);
	spec = pow(spec, 14.0);
	
	
	// Add diffuse and specular
	phongResult += shadow * ((diff * kd) + (spec * ks)) * kt;
	//vec4 phongResult = vec4(max(dot(vertNormal, lightDir), 0));
	fragColor = phongResult;
	//fragColor = shadow * vec4(1.0,1.0,1.0,1.0);
} 

