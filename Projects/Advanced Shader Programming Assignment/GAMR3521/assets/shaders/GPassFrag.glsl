#version 460 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gDiffSpec;
layout(location = 3) out vec4 gRoughMetAO;

in vec3 fragmentPos;
in vec2 texCoord;
in mat3 TBN;

layout (std140, binding = 0) uniform b_camera
{
	uniform mat4 u_view;
	uniform mat4 u_projection;
	uniform vec3 u_viewPos;
};

uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;

vec3 normal;
float metallicVal;
float roughnessVal;

void main()
{	
	vec3 albedoColour = texture(u_albedoMap, texCoord).rgb;

	vec3 N = texture(u_normalMap, texCoord).rgb;
	normal = normalize(TBN * (N * 2.0 - 1.0));
	metallicVal = texture(u_metallicMap, texCoord).r;
	roughnessVal = texture(u_roughnessMap, texCoord).r;
	
	gPosition = vec4(fragmentPos, 1.0);
	gNormal = vec4(normal, 1.0);
	gDiffSpec = vec4(albedoColour, 1.0);
	gRoughMetAO = vec4(roughnessVal, metallicVal, 1.0, 1.0);
}