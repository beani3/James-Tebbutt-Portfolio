#version 460 core

#extension GL_ARB_bindless_texture : require

// Struct for Vertex GPU-side
struct Vertex {
	vec4 colour;
	float position[2];
	float uv[2];
};

// Buffers
layout (std430, binding = 0) readonly buffer vertexSSBO {
	Vertex vertices[];
};

//layout (std140, binding = 1) uniform b_camera2D {
//	uniform mat4 view;
//	uniform mat4 projection;
//};

uniform mat4 view;
uniform mat4 projection;

out vec2 aUV;
out vec4 aCol;
flat out int fsInstance; // Needed to select which texture 

vec2 getPosition(int idx) {
	return vec2(vertices[idx].position[0], vertices[idx].position[1]);
}

vec2 getUV(int idx) {
	return vec2(vertices[idx].uv[0], vertices[idx].uv[1]);
}

void main() {
	int idx = gl_VertexID + (gl_InstanceID * 4); // Each instance has 4 vertices
	vec2 pos = getPosition(idx);
	gl_Position = projection * view * vec4(pos, 1.0, 1.0);

	aUV = getUV(idx);
	aCol = vertices[idx].colour;
	fsInstance = gl_InstanceID;
}