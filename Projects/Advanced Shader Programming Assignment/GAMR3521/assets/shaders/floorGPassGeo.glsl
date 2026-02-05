#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 tes_fragmentPos[];
in vec2 tes_texCoord[];
in vec4 tes_fragmentPosLightSpace[];
in vec3 tes_CDMnormal[];

out vec2 texCoord;
out vec3 fragmentPos;
out vec3 vertexNormal;
out vec3 CDMnormal;
out mat3 TBN;

vec3 getNormal() {
	vec3 a = tes_fragmentPos[0] - tes_fragmentPos[1];
	vec3 b = tes_fragmentPos[0] - tes_fragmentPos[2];
	return normalize(cross(a, b));;
}

vec3 getTangent() {
	vec2 deltaUV1  = tes_texCoord[0] - tes_texCoord[1];	
	vec2 deltaUV2  = tes_texCoord[0] - tes_texCoord[2];	
	vec3 deltaPos1 = tes_fragmentPos[0] - tes_fragmentPos[1];
	vec3 deltaPos2 = tes_fragmentPos[0] - tes_fragmentPos[2];

	float r = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

	vec3 thisTan = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
	return thisTan;
	//return vec3(1.0, 0.0, 0.0);
}

vec3 getBitangent() {
	vec2 deltaUV1  = tes_texCoord[0] - tes_texCoord[1];	
	vec2 deltaUV2  = tes_texCoord[0] - tes_texCoord[2];	
	vec3 deltaPos1 = tes_fragmentPos[0] - tes_fragmentPos[1];
	vec3 deltaPos2 = tes_fragmentPos[0] - tes_fragmentPos[2];

	float r = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

	vec3 thisBiTan = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
	return thisBiTan;
	//return vec3(0.0, 0.0, 1.0);
}

void main() {
	vec3 T = normalize(getTangent());
	vec3 B = normalize(getBitangent());

	for (int i = 0; i < 3; i++) {
		texCoord = tes_texCoord[i];
		vertexNormal = getNormal();
		CDMnormal = tes_CDMnormal[i];
		fragmentPos = tes_fragmentPos[i];
		TBN = mat3(T, B, vertexNormal);
		gl_Position = gl_in[i].gl_Position;
	
		EmitVertex();
	}

	EndPrimitive();
}