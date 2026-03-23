#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL

layout(location = 0) out vec4 fragColor;

// base on the presentaion "Next Generation Post Processing in Call of Duty Advanced Warfare" by Jorge Jimenez slide 144
layout(location = 0) in vec2 uv;


layout( set = 0, binding = 0 ) uniform sampler2D srcSampler;
// layout(set = 0, binding = 1) uniform sampler2D bloomSampler; // add later
//Just for testing if HDR frame are working.
void main() { 
	vec2 texelSize = 1.0 / vec2(textureSize(srcSampler, 0));
    float x = texelSize.x;
    float y = texelSize.y;
    vec4 result = vec4(0.0);

    vec4 tl = texture(srcSampler,vec2(uv.x - x,uv.y + y));
    vec4 ct = texture(srcSampler,vec2(uv.x,uv.y + y));
    vec4 tr = texture(srcSampler,vec2(uv.x + x,uv.y + y));
    vec4 cl = texture(srcSampler,vec2(uv.x - x,uv.y));
    vec4 c = texture(srcSampler,vec2(uv.x,uv.y));
    vec4 cr = texture(srcSampler,vec2(uv.x + x,uv.y));
    vec4 bl = texture(srcSampler,vec2(uv.x - x,uv.y - y));
    vec4 cb = texture(srcSampler,vec2(uv.x,uv.y - y));
    vec4 br = texture(srcSampler,vec2(uv.x + x,uv.y - y));
   
   result += (tl + tr + bl + br ) * ( 1.0 / 16.0 );
   result += (ct + cl + cr + cb ) * ( 2.0 / 16.0 );
   result += (c ) * ( 4.0 / 16.0 );


   fragColor = vec4(result.rgb, 1.0);
} 

