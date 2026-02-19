#version 450 core

layout(location = 0) out vec4 dofColour;

in vec2 texCoord;

uniform sampler2D u_inputTexture;
uniform sampler2D u_blurTexture;
uniform sampler2D u_depthTexture;

void main() {
	vec3 rgb = texture(u_inputTexture, texCoord).rgb;
	vec3 blur = texture(u_blurTexture, texCoord).rgb;
	float depthValue = texture(u_depthTexture, texCoord).r;

	vec3 result = mix(rgb, blur, depthValue);
	dofColour = vec4(result, 1.0);
}