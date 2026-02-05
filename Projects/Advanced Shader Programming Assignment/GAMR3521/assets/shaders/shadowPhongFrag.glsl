#version 450 core

layout(location = 0) out vec4 shadowColour;

in vec2 texCoord;

uniform sampler2D u_inputTexture;


void main() {
	vec3 rgb = texture(u_inputTexture, texCoord).rgb;
	shadowColour = vec4(rgb, 1.0);
}