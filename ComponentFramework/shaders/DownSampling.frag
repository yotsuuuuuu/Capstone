#version 450
#extension GL_ARB_separate_shader_objects : enable // need for openGL


layout(location = 0) in vec2 uv;


layout(location = 0) out vec4 fragColor;

// base on the presentaion "Next Generation Post Processing in Call of Duty Advanced Warfare" by Jorge Jimenez slide 144

layout( set = 0, binding = 0 ) uniform sampler2D srcSampler;
// layout(set = 0, binding = 1) uniform sampler2D bloomSampler; // add later
//Just for testing if HDR frame are working.
void main() { 
	vec2 texelSize = 1.0 / vec2(textureSize(srcSampler, 0));
    float x = texelSize.x;
    float y = texelSize.y;
    vec4 result = vec4(0.0);
    
     // dead center
    vec4 o = texture(srcSampler,uv);

    // center box
    vec4 ctr = texture(srcSampler, vec2(uv.x + x, uv.y + y)); // top right
    vec4 ctl = texture(srcSampler, vec2(uv.x - x, uv.y + y)); // top left
    vec4 cbr = texture(srcSampler, vec2(uv.x + x, uv.y - y)); // bottom right
    vec4 cbl = texture(srcSampler, vec2(uv.x - x, uv.y - y)); // bottom left

    // outer 9 samples
    vec4 o1 = texture(srcSampler,vec2(uv.x - 2 * x,uv.y + 2 * y )); // top left outer
    vec4 o2 = texture(srcSampler,vec2(uv.x + 2 * x,uv.y + 2 * y )); // top right outer
    vec4 o3 = texture(srcSampler,vec2(uv.x + 2 * x,uv.y - 2 * y )); // bottom right outer
    vec4 o4 = texture(srcSampler,vec2(uv.x - 2 * x,uv.y - 2 * y )); // bottom left outer
    vec4 o5 = texture(srcSampler,vec2(uv.x,        uv.y + 2 * y )); // top center edge
    vec4 o6 = texture(srcSampler,vec2(uv.x + 2 * x,uv.y         )); // right center edge
    vec4 o7 = texture(srcSampler,vec2(uv.x,        uv.y - 2 * y )); // bottom center edge
    vec4 o8 = texture(srcSampler,vec2(uv.x - 2 * x,uv.y         )); // left center edge

    float centerweight  = 0.5;
    float connerweight  = 0.125;
    result += (ctr + ctl +cbr + cbl) * centerweight;
    result += (o + o8 + o1 + o5) * connerweight;
    result += (o + o5 + o2 + o6) * connerweight;
    result += (o + o6 + o3 + o7) * connerweight;
    result += (o + o7 + o4 + o8) * connerweight;



    fragColor = vec4(result.rgb, 1.0);     

} 

