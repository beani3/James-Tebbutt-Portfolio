#version 460 core

layout(location = 0) out vec4 colour;

in vec2 texCoord;
in mat3 TBN;

struct directionalLight
{
	vec3 colour;
	vec3 direction;
};

struct pointLight
{
	vec3 colour;
	vec3 position;
	vec3 constants;
};

struct spotLight
{
	vec3 colour;
	vec3 position;
	vec3 direction;
	vec3 constants;
	float cutOff;
	float outerCutOff;
};

const int numPointLights = 50;
const int numSpotLights = 1;

layout (std140, binding = 1) uniform b_lights
{
	uniform directionalLight dLight;
	uniform pointLight pLights[numPointLights];
	uniform spotLight sLights[numSpotLights];
};

uniform vec3 u_viewPos;

uniform sampler2D u_gBuffPosInWS;
uniform sampler2D u_gBuffNormal;
uniform sampler2D u_gBuffDiffSpec;
uniform sampler2D u_gBuffRoughMetAO;
uniform sampler2D u_depthAttachment;
uniform sampler2D u_skyboxCol;

// forward declare
vec3 getDirectionalLight() ;
vec3 getPointLight(int idx) ;
vec3 getSpotLight(int idx) ;
// PBR functions
vec3 freshnelSchlick(float cosTheta, vec3 F0) ;
float distributionGGX(float NdotH);
float geometrySchickGGX(float Ndot);
float geometrySmith();

//  Shadow uniforms
uniform sampler2D u_shadowMap;
uniform mat4 u_lightSpaceTransform;

// PBR Uniforms;
uniform float u_ambientFactor;
uniform float u_metallic;
uniform float u_roughness;

// PBR variables
vec3 V;
vec3 F0;
float NdotL;
float NdotH;
float NdotV;
float VdotH;
const float PI = 3.14159;

vec3 posInWS;
vec3 albedo;
vec3 normal;
float metallic;
float roughness;

vec4 posInLS;

float shadowCalculation() 
{
	vec3 projCoords = posInLS.xyz / posInLS.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(u_shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float shadow;

	float bias = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
	    {
	        float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
	        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
	    }    
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0) { 
		shadow = 0.0; 
	}

	return shadow;
}

void main()
{	
	vec2 uv = texCoord;
	posInWS = texture(u_gBuffPosInWS, uv).rgb;

	normal = texture(u_gBuffNormal, uv).rgb;
	V = normalize(u_viewPos - posInWS);
	
	roughness = (texture(u_gBuffRoughMetAO, uv).r) * u_roughness; 
	metallic = (texture(u_gBuffRoughMetAO, uv).g) * u_metallic;
	albedo = texture(u_gBuffDiffSpec, uv).rgb;
	// albedo = vec3(1.0);
	F0 = mix(vec3(0.4), albedo, metallic);

	posInLS = u_lightSpaceTransform * vec4(posInWS, 1.0);
	float depthValue = texture(u_depthAttachment, uv).r;
	vec3 skyboxCol = texture(u_skyboxCol, uv).rgb;

	vec3 result = vec3(0.0); 

	result += getDirectionalLight();
	
	for(int i = 0; i <numPointLights; i++)
	{
		result += getPointLight(i);
	}
	
	for(int i = 0; i <numSpotLights; i++)
	{
		//result += getSpotLight(i);
	}

	// colour = vec4(result, 1.0) * vec4(albedo, 1.0);
	colour = vec4(result, 1.0);
	if (depthValue >= 0.9999f) colour = vec4(skyboxCol, 1.0);
}

vec3 getDirectionalLight()
{
	vec3 L = normalize(-dLight.direction);
	vec3 H = normalize(L + V);

	NdotL = max(dot(normal, L), 0.0001);
	NdotH = max(dot(normal, H), 0.0001);
	NdotV = max(dot(normal, V), 0.0001);
	VdotH = max(dot(V, H), 0.0001);

	vec3 F = freshnelSchlick(VdotH, F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;
	// float diff = max(dot(normal, -dLight.direction), 0.0);
	// vec3 diffuse = diff * albedo * dLight.colour;
	vec3 diffuse = (kD * albedo / PI) * NdotL;

	// float shadowAmount = 0.5;
	float shadowAmount = shadowCalculation();
	// vec3 ambient = ambientStrength * dLight.colour;
	vec3 ambient = u_ambientFactor * vec3(1.0) * albedo;
	vec3 viewDir = normalize(u_viewPos - posInWS);
	vec3 reflectDir = reflect(dLight.direction, normal);  

	float D = distributionGGX(NdotH);
	float G = geometrySmith();
	vec3 numerator = D * G * F;
	float denominator = 4.0 * NdotV * NdotL + 0.0001;
	// vec3 specular = specularStrength * spec * dLight.colour; 
	vec3 specular = numerator / denominator;   

	return ambient + (1.0 - shadowAmount) * (specular + diffuse) * (dLight.colour);
	// return diffuse;
	// return vec3(D);
	// return vec3(geometrySchickGGX(NdotL));
	/// return vec3(geometrySchickGGX(NdotV));
}

vec3 getPointLight(int idx)
{
	// vec3 ambient = ambientStrength * pLights[idx].colour;
	vec3 ambient = u_ambientFactor * vec3(1.0) * albedo;
	float distance = length(pLights[idx].position - posInWS);
	float attn = 1/(pLights[idx].constants.x + (pLights[idx].constants.y* distance) + (pLights[idx].constants.z * (distance * distance)));
	vec3 lightDir = normalize(pLights[idx].position - posInWS);

	vec3 L = normalize(-dLight.direction);
	vec3 H = normalize(L + V);

	NdotL = max(dot(normal, L), 0.0001);
	NdotH = max(dot(normal, H), 0.0001);
	NdotV = max(dot(normal, V), 0.0001);
	VdotH = max(dot(V, H), 0.0001);

	vec3 F = freshnelSchlick(VdotH, F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 diff = (kD * albedo / PI) * NdotL;
	vec3 diffuse = diff * attn * pLights[idx].colour;
	vec3 viewDir = normalize(u_viewPos - posInWS);
	vec3 reflectDir = reflect(-lightDir, normal); 
	
	float D = distributionGGX(NdotH);
	float G = geometrySmith();
	vec3 numerator = D * G * F;
	float denominator = 4.0 * NdotV * NdotL + 0.0001;
	// vec3 specular = specularStrength * spec * attn * pLights[idx].colour;  
	vec3 specular = (numerator / denominator) * attn * pLights[idx].colour;
	
	return ambient + (specular + diffuse);
	// return diffuse;
	// return vec3(D);
	// return vec3(geometrySchickGGX(NdotL));
	// return vec3(geometrySchickGGX(NdotV));
}

vec3 getSpotLight(int idx)
{	
	vec3 lightDir = normalize(sLights[idx].position - posInWS);
	float theta = dot(lightDir, normalize(-sLights[idx].direction));
	float ambientStrength = 0.4;
	vec3 ambient = ambientStrength * sLights[idx].colour;
	if(theta > sLights[idx].outerCutOff)
	{
		vec3 norm = normalize(normal);
		float distance = length(sLights[idx].position - posInWS);
		float attn = 1.0/(sLights[idx].constants.x + (sLights[idx].constants.y* distance) + (sLights[idx].constants.z * (distance * distance)));
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * albedo * attn * sLights[idx].colour;
		vec3 viewDir = normalize(u_viewPos - posInWS);
		vec3 reflectDir = reflect(-lightDir, norm);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
	
		float epsilon = sLights[idx].cutOff - sLights[idx].outerCutOff;
		float intensity = clamp((theta - sLights[idx].outerCutOff) / epsilon, 0.0, 1.0); 
	
		
		return (ambient + diffuse);
	}
	else
	{
		return ambient;
	}
}

vec3 freshnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float distributionGGX(float NdotH) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH2 = NdotH * NdotH;

	float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
	denominator = PI * denominator * denominator;

	return a2 / denominator;
}

float geometrySchickGGX(float Ndot) {
	float a = roughness * roughness;
	float r = (a + 1.0);
	float k = (r * r) / 8.0;
	float denominator = Ndot * (1.0 - k) + k;

	return Ndot / denominator;
}

float geometrySmith() {
	float ggx2 = geometrySchickGGX(NdotV); // View Direction
	float ggx1 = geometrySchickGGX(NdotL); // Light Direction

	return ggx1 * ggx2;
}