#version 460 core

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba16f) uniform image2D noiseImg;

// Noise Functions
float remap(float currValue, float  inMin,float inMax,float outMin, float outMax);
vec2 hash2(vec2 p);
float noise(vec2 p);
float FBM(vec2 pos);
float ridgedFBM(vec2 pos);

// Noise uniforms and variables
uniform int u_octaves;
uniform float u_frequency;
uniform float u_amplitude;
uniform float u_lacunarity;
uniform float u_persistance;
float p;

void main() {

    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	// Convert to UV coordinates
	vec2 uv = vec2(pixel_coords) / vec2(512);

	// p = noise(uv * u_frequency) * u_amplitude;
	
    p = FBM(uv * u_frequency);
	p *= u_amplitude;

	// p = ridgedFBM(uv * u_frequency);
	// p *= u_amplitude;

	p = remap(p, -1.0, 1.0, 0.0, 1.0);

    imageStore(noiseImg, pixel_coords, vec4(vec3(p, p, p), 1.0));
}

float remap(float currValue, float  inMin,float inMax,float outMin, float outMax) {
	
	float t = (currValue - inMin) / (inMax - inMin);
	return mix(outMin, outMax, t);
}

vec2 hash2(vec2 p) 
{
	p = vec2(dot(p, vec2(127.1, 311.7)),
		dot(p, vec2(269.5, 183.3)));

	return -1.0 + 2.0 * fract(sin(p) * 43758.5453123 );
}

float noise(vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);

	vec2 u = f * f * (3.0 - 2.0 * f);

	return mix(mix(dot(hash2(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0)),
		dot(hash2(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u.x),
		mix(dot(hash2(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
			dot(hash2(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u.x), u.y);
}

float FBM(vec2 pos) {
	float total = 0.0;
	float totalAmplitude = 0.0;
	float frequency = u_frequency;
	float amplitude = u_amplitude;

	for (int i = 0; i < u_octaves; i++) {
		total += noise(pos * frequency) * amplitude;
		totalAmplitude += amplitude;
		frequency *= u_lacunarity;
		amplitude *= u_persistance;
	}

	return (total / totalAmplitude);
}

float ridgedFBM(vec2 pos) {
	float total = 0.0;
	float totalAmplitude = 0.0;
	float frequency = u_frequency;
	float amplitude = u_amplitude;

	for (int i = 0; i < u_octaves; i++) {
		float noiseVal = noise(pos * frequency);
		noiseVal = abs(noiseVal);
		noiseVal = 1.0 - noiseVal;
		noiseVal = sqrt(noiseVal);

		total += noiseVal * amplitude;
		totalAmplitude += amplitude;
		frequency *= u_lacunarity;
		amplitude *= u_persistance;
	}

	return (total / totalAmplitude);
}