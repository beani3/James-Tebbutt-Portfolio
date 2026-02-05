#version 460 core

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba16f) uniform image2D cdmImg;

uniform sampler2D u_noiseTexture;

void main() {

    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	// Convert to UV coordinates
	vec2 uv = vec2(pixel_coords) / vec2(512);

	float scale = 100;

	float left = textureOffset(u_noiseTexture, uv, ivec2(-1, 0)).r * scale;
	float right = textureOffset(u_noiseTexture, uv, ivec2(1, 0)).r * scale;
	float up = textureOffset(u_noiseTexture, uv, ivec2(0, -1)).r * scale;
	float down = textureOffset(u_noiseTexture, uv, ivec2(0, 1)).r * scale;

	float lr = left - right;
	float ud = up - down;

	vec3 cdmNormal = normalize(vec3(lr, 2.0, ud));
	float heightVal = texture(u_noiseTexture, uv).r;

    imageStore(cdmImg, pixel_coords, vec4(cdmNormal, heightVal));
}