#version 460 core

layout (vertices = 3) out;

in vec3 vs_fragmentPos[];
in vec2 vs_texCoord[];
in vec4 vs_fragmentPosLightSpace[];

out vec3 tcs_fragmentPos[];
out vec2 tcs_texCoord[];
out vec4 tcs_fragmentPosLightSpace[];

layout (std140, binding = 0) uniform b_camera
{
	uniform mat4 u_view;
	uniform mat4 u_projection;
	uniform vec3 u_viewPos;
};

uniform mat4 u_model;

float getTessLevel(float dist0, float dist1);

void main() {
	float tessLevel = 1.0;
	float eyeToVertexDist0 = distance(u_viewPos, vs_fragmentPos[0]);
	float eyeToVertexDist1 = distance(u_viewPos, vs_fragmentPos[1]);
	float eyeToVertexDist2 = distance(u_viewPos, vs_fragmentPos[2]);

	if (gl_InvocationID == 0) {
		
		// Calc tesellation levels
		gl_TessLevelOuter[0] = getTessLevel(eyeToVertexDist1, eyeToVertexDist2);
		gl_TessLevelOuter[1] = getTessLevel(eyeToVertexDist2, eyeToVertexDist0);
		gl_TessLevelOuter[2] = getTessLevel(eyeToVertexDist0, eyeToVertexDist1);
		gl_TessLevelInner[0] = gl_TessLevelOuter[2];
	}

	tcs_fragmentPos[gl_InvocationID] = vs_fragmentPos[gl_InvocationID];
	tcs_texCoord[gl_InvocationID] = vs_texCoord[gl_InvocationID];
	tcs_fragmentPosLightSpace[gl_InvocationID] = vs_fragmentPosLightSpace[gl_InvocationID];
}

float getTessLevel(float dist0, float dist1) {
	float avgDist = (dist0 + dist1) / 2.0;

	float minTess = 4.0;
	float maxTess = 32.0;
	float nearDist = 10.0;
	float farDist = 100.0;

	float t = clamp((avgDist - nearDist) / (farDist - nearDist), 0.0, 1.0);

	// Invert so closer = more tessellation;
	return mix(maxTess, minTess, t);
}