#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

#define MAX_SHADOW_MAPS 3

layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 fragTexCoords;
layout (location = 3) in vec3 lightDir;
layout (location = 4) in vec4 fragLightSpace[MAX_SHADOW_MAPS];

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



float ShadowCheck(int index, int sampleSize);


bool insideCascade(int index);

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
	bool isInCasede0 = insideCascade(0);
	bool isInCasede1 = insideCascade(1);
	bool isInCasede2 = insideCascade(2);
	 
	float shadow  = 1.0;

	if(isInCasede0 && isInCasede1 && isInCasede2  ) {
		shadow = ShadowCheck(0,1);
		if(shadow >= 0.01){
			float medshadow = ShadowCheck(1,2);
			if(medshadow < 1.0){
			shadow = (medshadow < shadow) ? medshadow : shadow;
			}else {
				float lowshadow = ShadowCheck(2,2);
				shadow = (lowshadow < shadow)? lowshadow : shadow;
			}
		}

	}else if( !(isInCasede0) &&  (isInCasede1) && !(isInCasede2) ){
		shadow = ShadowCheck(1,1);
	}	
	else if (!(isInCasede0) && (isInCasede1) && (isInCasede2) ) {
		float s = ShadowCheck(1,1);
		float s1 = ShadowCheck(2,1);
		shadow = min(s,s1);
	}
	else if (!(isInCasede0) &&  !(isInCasede1) && (isInCasede2)) {
		shadow = ShadowCheck(2,1);
	}


	//float shadow = 1.0; // Temporary - no shadows
	
	diff = max(dot(vertNormal, lightDir), 0);	
	reflection = normalize(reflect(-lightDir, vertNormal));
	spec = max(dot(eyeDir, reflection), 0.0);
	spec = pow(spec, 14.0);
	spec *= diff > 0.0 ? 1.0 : 0.0; // small fix but dosen't to much in the grand scheme of things
	
	// Add diffuse and specular
	phongResult += (shadow * ((diff * kd) + (spec * ks)) ) * kt;
	//vec4 phongResult = vec4(max(dot(vertNormal, lightDir), 0));
	fragColor = phongResult;
	//fragColor = shadow * vec4(1.0,1.0,1.0,1.0);
} 


float ShadowCheck(int index, int sampleSize) {

	// going form clip space to NDC space
	vec4 pos = fragLightSpace[index];
	float invW = 1.0 / pos.w;
	
	vec3 ndc = pos.xyz * invW;
	// Vulkan ndc  -1 to 1 for x and y and z form 0 - 1
	// so need to remap to texture jsut x and y
	vec2 uv =  ndc.xy * 0.5 + 0.5;

	float minBias = 0.001;
	float slopeFactor = 0.015;
	float NL = max( dot(normalize(vertNormal), normalize(lightDir)) ,0.0);
	float bias = max(minBias,slopeFactor * (1 - NL));
	float depth = ndc.z - bias;
	float clampedDepth = clamp(depth, 0.0, 1.0);
	vec2 texelSize = 1.0 / vec2(textureSize(shadowMap[index], 0));
	
	int kernelHalf = sampleSize;

	int totalSamples = 0;
	float result = 0.0;
	for(int x = -kernelHalf; x<= kernelHalf ;x++){
		for(int y = -kernelHalf; y<= kernelHalf ;y++){
			vec2 offset = vec2(x,y)* texelSize;
			result += texture(shadowMap[index], vec3(uv + offset, clampedDepth));
			totalSamples++;
		}
	}


    return (totalSamples == 0 ) ?   texture(shadowMap[index], vec3(uv , clampedDepth)) : result /float(totalSamples);

	
}


bool insideCascade(int index)
{
    vec4 clip = fragLightSpace[index];

    if (clip.w <= 0.0)
        return false;

    vec3 ndc = clip.xyz / clip.w;

    float eps = 0.1;

    return abs(ndc.x) < 1.0 - eps &&
           abs(ndc.y) < 1.0 - eps &&
           ndc.z > 0 + eps &&
           ndc.z < 1.0 - eps;
}