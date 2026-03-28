#version 450

uniform sampler2D tex;
uniform float mipLevel;

in vec2 a_UV;
layout(location = 0) out vec4 colour;

void main() {
    colour = vec4(1,0,0,1);
    if(mipLevel == 0.0) colour = textureLod(tex, gl_FragCoord.xy / vec2(711.0, 400.0),mipLevel);
    if(mipLevel == 1.0) colour = textureLod(tex, gl_FragCoord.xy / vec2(711.0, 400.0),mipLevel);
    if(mipLevel == 2.0) colour = textureLod(tex, gl_FragCoord.xy / vec2(355.0, 200.0),mipLevel);
    if(mipLevel == 3.0) colour = textureLod(tex, gl_FragCoord.xy / vec2(178.0, 100.0),mipLevel);
    if(mipLevel == 4.0) colour = textureLod(tex, gl_FragCoord.xy / vec2(89.0, 50.0),mipLevel);
    if(mipLevel == 5.0) colour = textureLod(tex, gl_FragCoord.xy / vec2(45.0, 25.0),mipLevel);
}