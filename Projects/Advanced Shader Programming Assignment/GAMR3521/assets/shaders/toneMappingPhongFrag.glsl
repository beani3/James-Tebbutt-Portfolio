#version 450 core

layout(location = 0) out vec4 finalColour;

in vec2 texCoord;

uniform sampler2D u_inputTexture;

// aces tone mapping
vec3 aces(vec3 x) {
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;

	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
	vec3 rgb = texture(u_inputTexture, texCoord).rgb;
	rgb = aces(rgb) ;					// Apply tone mapping
	// rgb = pow(rgb, vec3(1.0 / 2.2));	// Apply gamma correction with gamma 2.2
	// rgb =  clamp(rgb, 0.0, 1.0);
	finalColour = vec4(rgb, 1);
}