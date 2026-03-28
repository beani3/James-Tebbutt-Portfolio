#version 460

layout(location = 0) out vec4 colour;
in vec2 texCoord;

	uniform sampler2D u_previousMipTex; 
	uniform sampler2D u_currentMipTex;  
	uniform vec2 u_texelSize;
	uniform int u_previousMip;
	uniform int u_currentMip;

float tentKernel[9] = {
	1.f/16.f, 2.f/16.f, 1.f/16.f,
	2.f/16.f, 4.f/16.f, 2.f/16.f,
	1.f/16.f, 2.f/16.f, 1.f/16.f
};

void main() {
	vec3 blurredPixel = vec3(0.0);
	int idx = 0;
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			vec2 offset = vec2(x, y) * u_texelSize;
			blurredPixel += textureLod(u_previousMipTex, texCoord + offset, float(u_previousMip)).rgb * tentKernel[idx++];
		}
	}
	vec3 currentPixel = texture(u_currentMipTex, texCoord, u_currentMip).rgb;
	colour = vec4(blurredPixel + currentPixel, 1.0);
}