#version 460 core

layout (triangles, equal_spacing, ccw) in;

in vec3 tcs_fragmentPos[];
in vec2 tcs_texCoord[];
in vec4 tcs_fragmentPosLightSpace[];

out vec3 tes_fragmentPos;
out vec2 tes_texCoord;
out vec4 tes_fragmentPosLightSpace;
out vec3 tes_CDMnormal;

layout (std140, binding = 0) uniform b_camera
{
	uniform mat4 u_view;
	uniform mat4 u_projection;
	uniform vec3 u_viewPos;
};

uniform sampler2D u_heightMap;
uniform float u_sf;

// Linear Interpolation
vec2 interpolation2D(vec2 v0, vec2 v1, vec2 v2) {
	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolation3D(vec3 v0, vec3 v1, vec3 v2) {
	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
} 

void main() {
	
	tes_fragmentPos = interpolation3D(tcs_fragmentPos[0], tcs_fragmentPos[1], tcs_fragmentPos[2]);
	tes_fragmentPosLightSpace.rgb = interpolation3D(tcs_fragmentPosLightSpace[0].rgb, tcs_fragmentPosLightSpace[1].rgb, tcs_fragmentPosLightSpace[2].rgb);
	tes_texCoord = interpolation2D(tcs_texCoord[0], tcs_texCoord[1], tcs_texCoord[2]);

	// CDM normal texture hold the normals in rgb and the height values in the alpha channel
	tes_CDMnormal = texture(u_heightMap, tes_texCoord).rgb;
	float height = texture(u_heightMap, tes_texCoord).a;

	tes_fragmentPos.y = height * u_sf;
	gl_Position = u_projection * u_view * vec4(tes_fragmentPos, 1.0);
}