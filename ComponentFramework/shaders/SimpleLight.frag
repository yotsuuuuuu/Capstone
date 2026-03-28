#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL




layout (location = 0) flat in int lightIndex;


layout (location = 0) out vec4 fragColor;

struct PointLight
{
    vec4 position_radius;
    vec4 color_intensity;
    vec4 direction_inner;
    vec4 outer_type_emissiveScale_audioID;
};

layout(std430, set = 0 , binding = 5) readonly restrict buffer lightSSBO
{
    PointLight pointLight[];
};

layout(set = 0, binding = 6) uniform Audio_SYS {
    mat4 bands;
} Audio_data;

layout(set = 1, binding = 0) uniform sampler2D texSampler;


float getAudioBand(int index) {
    int col = index / 4;
    int row = index % 4;
    return Audio_data.bands[col][row];
}

void main() { 
	 PointLight light = pointLight[lightIndex];

    vec3 color = light.color_intensity.xyz;
    int AudioIndex = int(light.outer_type_emissiveScale_audioID.w);
    float intensityMod = (AudioIndex != -1) ? getAudioBand(AudioIndex) * 0.5 : 0.0;

    float intensity = light.outer_type_emissiveScale_audioID.z + intensityMod;
    
    float colorLuminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float luminanceScale = 1.0 / max(colorLuminance, 0.0001);
    
    float whiteBias = 0.4;
    vec3 emissive = color * intensity * luminanceScale;
    fragColor = vec4(mix(emissive, vec3(1.0), whiteBias), 1.0);  
} 

