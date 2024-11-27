#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aTan;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 normal;
out vec3 posInWS;
out vec2 uv;
out vec3 tan;

out mat3 TBN;

void main() {
	uv = aUV;
	tan = aTan;
	normal = (Model * vec4(aNorm, 0.0)).xyz;
	vec4 worldSpace = Model * vec4(aPos, 1.0);
	posInWS = worldSpace.xyz;
	gl_Position = Projection * View * worldSpace;

	vec3 T = (Model * vec4(aTan, 0.0)).xyz;
	vec3 N = normal;
	vec3 B = normalize(cross(T, N));
	TBN = mat3(T, B, N);
}