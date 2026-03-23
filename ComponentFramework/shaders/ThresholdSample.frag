#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 uv;
layout(location = 1) flat in float threshold;
layout(location = 2) flat in float strength;



layout( set = 0, binding = 0 ) uniform sampler2D hdrSampler;

void main() { 
	vec3 colour = texture(hdrSampler,uv).xyz;
	// optionanl luminace  options (0.299,0.587,0.114)
	// or  bit slower sqrt( 0.299*R^2 + 0.587*G^2 + 0.114*B^2 )
	float luminance = dot(colour, vec3(0.2126, 0.7152, 0.0722));
	vec3 result = (luminance > threshold) ? colour : vec3(0.0);
    fragColor = vec4(result, 1.0);
} 

