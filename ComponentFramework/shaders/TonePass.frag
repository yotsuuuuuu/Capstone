#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 uvcoord;
layout(location = 1) flat in float threshold;
layout(location = 2) flat in float bloomStrength;


layout( set = 0, binding = 0 ) uniform sampler2D hdrSampler;
layout( set = 0, binding = 1 ) uniform sampler2D bloomSampler; 
//Just for testing if HDR frame are working.
void main() { 
	vec3 hdr = texture(hdrSampler, uvcoord).rgb;
	vec3 bloom = texture(bloomSampler, uvcoord).rgb;

	vec3 combined = hdr + bloom * bloomStrength;

	//tone mapping 
	vec3 mapped = combined / (combined + vec3(1.0)); // Reinhard

	fragColor = vec4(mapped, 1.0);
} 

