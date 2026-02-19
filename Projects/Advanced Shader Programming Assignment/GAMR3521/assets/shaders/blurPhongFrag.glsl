#version 450 core

layout(location = 0) out vec4 ppColour;

in vec2 texCoord;

uniform sampler2D u_inputTexture;

// Blur uniforms
uniform int u_radius;
uniform vec2 u_imageSize;

void main() {
	vec4 colourSum = vec4(0.0);
	float sampleCount = 0.0;
	vec3 rgb = texture(u_inputTexture, texCoord).rgb;

	for (int x = -u_radius; x <= u_radius; ++x) {
		for (int y = -u_radius; y <= u_radius; ++y) {
			
			// Convert pixel offset to UV offsets
			vec2 samplePos = texCoord + vec2(x, y) / u_imageSize;

			// Clamp don't sample outside texture 
			samplePos = clamp(samplePos, vec2(0.0), vec2(1.0));

			colourSum += texture(u_inputTexture, samplePos);
			sampleCount += 1.0;
		}
	}

	ppColour = colourSum / sampleCount;
}