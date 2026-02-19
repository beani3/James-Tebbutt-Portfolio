#version 450 core

layout(location = 0) out vec4 Colour;

in vec2 texCoord;

uniform sampler2D u_inputTexture;
uniform sampler2D u_depthTexture;


void main() {
	vec3 rgb = texture(u_inputTexture, texCoord).rgb;
	vec3 fogColour = vec3(0.5);
	float depthValue = texture(u_depthTexture, texCoord).r;

	vec3 result = mix(rgb, fogColour, depthValue);
	Colour = vec4(result, 1);
}