#version 460 core

uniform vec3 viewPos;

// material uniforms
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform float shine;

// directional light uniforms
uniform vec3 lightColour;
uniform vec3 lightDirection;

uniform float ambientFactor;

// point light uniforms
struct PointLight {
	vec3 col;
	vec3 pos;
	vec3 attn;
};

#define numPL 50
uniform PointLight pointLight[numPL];

// spotlight uniforms
uniform vec3 slightPosition;
uniform vec3 slightColour;
uniform vec3 sAttentuation;
uniform vec3 slightDirection;
uniform vec2 sRadii;

out vec4 FragColor;

in vec3 normal;
in vec3 posInWS;
in vec2 uv;

vec3 n; 
vec3 viewDir; 

vec3 getDirectionalLight() {
	vec3 objColour = texture(diffuseMap, uv).rgb; 
	float specStrength = texture(specularMap, uv).r;

	// ambient
	vec3 ambient = objColour * lightColour * ambientFactor;
	vec3 lightDir = normalize(-lightDirection);
	// diffuse 
	float diffuseFactor = dot(n, lightDir);
	diffuseFactor = max(diffuseFactor, 0.0f);
	vec3 diffuse = objColour * lightColour * diffuseFactor;

	// Blinn Phong Specular 
	vec3 H = normalize(lightDir + viewDir);
	float specLevel = dot(n, H);
	specLevel = max(specLevel, 0.5);
	specLevel = pow(specLevel, shine);
	vec3 specular = lightColour * specLevel * specStrength;

	return ambient + diffuse + specular;
}

vec3 getPointLight(int i) {
	vec3 objColour = texture(diffuseMap, uv).rgb; 
	float specStrength = texture(specularMap, uv).r;

	// attn
	float distance = length(pointLight[i].pos - posInWS);
	float attn = 1.0 / (pointLight[i].attn.x + (pointLight[i].attn.y * distance) + (pointLight[i].attn.z * (distance * distance)));

	// diffuse 
	vec3 lightDir = normalize(-lightDirection);
	float diffuseFactor = dot(n, lightDir);
	diffuseFactor = max(diffuseFactor, 0.0f);
	vec3 diffuse = objColour * pointLight[i].col * diffuseFactor;

	// Blinn Phong Specular 
	vec3 H = normalize(lightDir + viewDir);
	float specLevel = dot(n, H);
	specLevel = max(specLevel, 0.0);
	specLevel = pow(specLevel, shine);
	vec3 specular = pointLight[i].col * specLevel * specStrength;

	diffuse * attn;
	specular * attn;

	return diffuse + specular;
}

vec3 getSpotLight() {
	vec3 objColour = texture(diffuseMap, uv).rgb; 
	float specStrength = texture(specularMap, uv).r;

	// attn
	float distance = length(slightPosition - posInWS);
	float attn = 1.0/(sAttentuation.x + (sAttentuation.y * distance) + (sAttentuation.z * (distance * distance)));
	vec3 lightDir = normalize((slightPosition - posInWS));

	// diffuse 
	float diffuseFactor = dot(n, lightDir);
	diffuseFactor = max(diffuseFactor, 0.0f);
	vec3 diffuse = objColour * slightColour * diffuseFactor;

	// Blinn Phong Specular 
	vec3 H = normalize(lightDir + viewDir);
	float specLevel = dot(n, H);
	specLevel = max(specLevel, 0.5);
	specLevel = pow(specLevel, shine);
	vec3 specular = slightColour * specLevel * specStrength ;

	diffuse = diffuse * attn;
	specular = specular * attn;

	// spotlight
	float theta = dot(-lightDirection, normalize(slightDirection));
	float denom = (sRadii.x - sRadii.y);
	float intensity = (theta - sRadii.y) / denom;
	intensity = clamp(intensity, 0.0, 1.0);
	diffuse = diffuse * intensity;
	specular = specular * intensity;

	return diffuse + specular;
}

void main() {
	n = normalize(normal);
	viewDir = normalize(viewPos - posInWS);
	vec3 result = getDirectionalLight();
	
	for (int i = 0; i < numPL; i++) {
		result += getPointLight(i);
	}

	result += getSpotLight();
	FragColor = vec4( result, 1.0);
}