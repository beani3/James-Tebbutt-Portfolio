#version 450 core

layout(location = 0) out vec4 depthColour;

in vec2 texCoord;

uniform sampler2D u_inputTexture; 
uniform float u_near;
uniform float u_far; 
 
float lineariseDepth(float depth) {
	float z = depth * 2.0 - 1.0; // Back to clip space
	return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}

void main() {
	float depthValue = texture(u_inputTexture, texCoord).r;
	float linearDepth = lineariseDepth(depthValue);
	float depthNormalized = (linearDepth - u_near) / (u_far - u_near);
	depthColour = vec4(vec3(depthNormalized), 1.0);
}