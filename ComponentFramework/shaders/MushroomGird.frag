#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

#define MAX_SHADOW_MAPS 3

layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 fragTexCoords;
layout (location = 3) in vec3 lightDir;
layout (location = 4) in vec4 fragViewPos;
layout (location = 5) in vec4 fragLightSpace[MAX_SHADOW_MAPS];

layout (location = 0) out vec4 fragColor;

layout(set = 0 , binding = 0) uniform CameraUBO {
    mat4 projectionMatrix;
	mat4 viewMatrix;
} Camera;

layout(set = 0, binding = 1) uniform GLightData{
	mat4 projectionMatrix[MAX_SHADOW_MAPS];
	mat4 viewMatrix[MAX_SHADOW_MAPS];
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 dir;
} GLData;

layout(set = 0, binding = 2) uniform sampler2DShadow shadowMap[MAX_SHADOW_MAPS];

layout(std140, set = 0 , binding = 3) uniform SYS_LIGHT_DATA {
    mat4 inverse_proj;
    uvec3 gridSize;
    uint _pad1; 
    uvec2 screenDimensions;
    float zNear;
    float zFar;
    uint lightCount;
    uint clusterCount;
    uint _pad2; 
    uint _pad3; 
} DataSys;

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(std430, set = 0, binding = 4) readonly restrict buffer clusterSSBO
{
    Cluster clusters[];
};

struct PointLight
{
    vec4 position_radius;
    vec4 color_intensity;
    vec4 direction_inner;
    vec4 outer_type_pad;
};

layout(std430, set = 0 , binding = 5) readonly restrict buffer lightSSBO
{
    PointLight pointLight[];
};


layout( set = 0 , binding = 7) uniform TerrainUBO
{
    vec4 min_max_lineWidth_edgeStrength;
    vec4 fadeStart_fadeEnd_gridScaleX_gridScaleY;
	vec4 ObjectFadeStart_ObjectFadeEnd_pad_pad;
    vec4 maxColor;
	vec4 minColor;
}TData;



layout(set = 1, binding = 0) uniform sampler2D texSampler;



float ShadowCheck(int index, int sampleSize);
bool insideCascade(int index);

vec4 ClusterLightsColour(vec4 ktColour);

vec4 GridColor(vec4 kt) ;

void main() { 
	vec3 reflection;
	float spec;
	float diff;
	vec4 ka = GLData.ambient;
	vec4 kd = GLData.diffuse;
	vec4 ks = GLData.specular;
	vec4 kt = vec4(0.02,0.02,0.02,1.0);
	vec4 gird_color = GridColor(kt);

	vec4 phongResult = ka * gird_color;
	// Ambient 
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
	vec4 specularC = spec * ks;

	// clamping both diff and specular
	float specLuminance = dot(specularC.rgb, vec3(0.2126, 0.7152, 0.0722));
	float specScale = min(0.5, 1.0 / max(specLuminance, 0.0001));
	specularC *= specScale;
	
	vec4 diffC = diff * kd;
	vec4 phongContrib = (diffC + specularC) * gird_color;
	
	
	float phongLuminance = dot(phongContrib.rgb, vec3(0.2126, 0.7152, 0.0722));
	float phongScale = min(0.8, 1.0 / max(phongLuminance, 0.0001));
	phongContrib *= phongScale;
	
	phongResult += shadow * phongContrib;
	phongResult += ClusterLightsColour(gird_color);
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

vec4 ClusterLightsColour(vec4 ktColour) {
	vec4 Result = vec4(0.0);
	
	uint zTile = uint((log(abs(fragViewPos.z) / DataSys.zNear) * float(DataSys.gridSize.z)) / log(DataSys.zFar / DataSys.zNear));
	vec2 tileSize = vec2(DataSys.screenDimensions) / vec2(DataSys.gridSize.xy);
    uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);
    uint tileIndex =
        tile.x + (tile.y *  DataSys.gridSize.x) + (tile.z *  DataSys.gridSize.x *  DataSys.gridSize.y);

    uint lightCount = clusters[tileIndex].count;
//	if (lightCount > 95) {
//     //getting close to limit. Output red color and dip
//     Result = vec4(1.0f, 0.0f, 0.0f, 1.0f);
//     return Result;
//	}

	for (int i = 0; i < lightCount; ++i)
    {
        uint lightIndex = clusters[tileIndex].lightIndices[i];
        PointLight light = pointLight[lightIndex];
       
	    // need to shift light pos to the right space
		vec4 LightInView = Camera.viewMatrix *  vec4(light.position_radius.xyz,1.0);
		vec3 pointLightPos  = LightInView.xyz;

        float radius     = light.position_radius.w;
        vec4 pointLightColor = vec4(light.color_intensity.xyz, 1.0);
        float intensity  = light.color_intensity.w;

		vec3 fragToLight = pointLightPos - fragViewPos.xyz;
		float dist = length(fragToLight);
		vec3 dirFragToLight = normalize(fragToLight);

		float attenuation = clamp(1.0 - (dist/radius),0.0,1.0);
		attenuation *= attenuation;

		float diff = max(dot(vertNormal, dirFragToLight), 0);	
		vec3 reflection = normalize(reflect(-dirFragToLight, vertNormal));
		float spec = max(dot(eyeDir, reflection), 0.0);
		spec = pow(spec, 14.0);
		spec *= diff > 0.0 ? 1.0 : 0.0;

		Result += intensity * attenuation * ((diff * pointLightColor) + ( spec * pointLightColor)) * ktColour;
		//Result += intensity  * ((diff * pointLightColor) + ( spec * pointLightColor)) * ktColour;
	    //Result = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		
    }


	
	return Result;
}



vec4 GridColor(vec4 kt) {
	

	float lineMod = 1.4f;

	vec2 gridScale = vec2(12,12); 
	vec2 grid = abs(fract(fragTexCoords * gridScale) - 0.5);
    float line = min(grid.x, grid.y);
    float edgeWidth = max(fwidth(line) * lineMod, 0.03);
    float gridLine = 1.0 - smoothstep(0.0, edgeWidth, line);
	vec4 edgeColor = texture(texSampler,fragTexCoords);
	float depth = length(fragViewPos);
	float fadeStart = TData.ObjectFadeStart_ObjectFadeEnd_pad_pad.x;
	float fadeEnd   = TData.ObjectFadeStart_ObjectFadeEnd_pad_pad.y;
    float depthFade = 1.0 - smoothstep(fadeStart, fadeEnd, depth);

    float edgeStrength = 0.6;
	float glowStrength = 0.8;
	
    // combine
    vec4 result = mix(kt, edgeColor, gridLine * edgeStrength * depthFade );

	result += edgeColor * gridLine * glowStrength * depthFade ;


	return result;
}