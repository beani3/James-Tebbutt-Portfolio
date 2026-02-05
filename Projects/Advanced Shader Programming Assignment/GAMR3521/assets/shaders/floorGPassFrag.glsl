#version 460 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gDiffSpec;
layout(location = 3) out vec4 gRoughMetAO;

in vec3 fragmentPos;
in vec2 texCoord;
in vec3 vertexNormal;
in vec3 CDMnormal;
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
uniform float u_sf;

// Toggle uniforms
uniform int u_textured;
uniform int u_normalToggle;

vec3 normal;
float roughnessVal;
float metallicVal;

void main()
{	
	vec3 n = texture(u_normalMap, texCoord).rgb;
	vec3 normal = normalize(TBN * (n * 2.0 - 1.0));

	vec3 col1 = vec3(0.2,0.55,0.2);
	vec3 col2 = vec3(0.6,0.2,0.1);

	vec3 albedoColour1 = mix(col1, col2, smoothstep(0.0, u_sf, fragmentPos.y));	
	vec3 albedoColour2 = texture(u_albedoMap, texCoord).rgb;
	vec3 albedoColour = mix(albedoColour1, albedoColour2, u_textured);

	gPosition = vec4(fragmentPos, 1.0);
	gNormal = vec4(vec3(0.0, 1.0, 0.0), 1.0);

	switch(u_normalToggle) {
	case 0:
		gNormal = vec4(vertexNormal, 1.0);
		break;
	case 1:
		gNormal = vec4(CDMnormal, 1.0);
		break;
	case 2:
		gNormal = vec4(normal, 1.0);
		break;
	default:
		break;

	}

	gDiffSpec = vec4(albedoColour, 1.0);

	roughnessVal = texture(u_roughnessMap, texCoord).r;
	metallicVal = texture(u_metallicMap, texCoord).g;

	gRoughMetAO = vec4(roughnessVal, metallicVal, 1.0, 1.0);
}

