#version 460 core
layout (local_size_x = 16, local_size_y = 16) in;

struct Particle {
	float age;
	vec4 position;
	vec4 velocity;
};
const int maxParticles = 10000;
Particle particles[maxParticles];

// Particle functions
float rand(vec2 co);
vec3 randomDirection(vec2 seed);

// Particle uniforms
uniform vec3 u_origin;
uniform float u_age;

void main() {
	
	vec2 gID = gl_GlobalInvocationID.xy;
	int gridWidth = 32; // 16 local x 2 global
	uint idx = uint(gID.x) * gridWidth + uint(gID.y);

	// Initialise particles
	particles[idx].position.xyz = u_origin;		// Particle position
	particles[idx].velocity.xyz = randomDirection(gID);	// Particle velocity; 
	particles[idx].age = u_age;					// Particle age;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 randomDirection(vec2 seed) {

    float theta = rand(seed) * 2.0 * 3.14159;
    float phi = acos(2.0 * rand(seed * 2.0) - 1.0);
    float x = sin(phi) * cos(theta) ;
    float y = sin(phi) * sin(theta);
    float z = cos(phi);
    return vec3(x, y, z);
}