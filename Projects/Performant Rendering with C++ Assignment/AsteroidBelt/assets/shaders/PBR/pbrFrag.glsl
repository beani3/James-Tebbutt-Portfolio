#version 460 core

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
const int numSpotLights = 2;

layout (std140, binding = 1) uniform b_lights
{
    uniform directionalLight dLight;
    uniform pointLight pLights[numPointLights];
    uniform spotLight sLights[numSpotLights];
};

layout (std140, binding = 0) uniform b_camera
{
    uniform mat4 u_view;
    uniform mat4 u_projection;
    uniform vec3 u_viewPos;
};

///////////////////////// INS OUTS
out vec4 FragColour;

in vec2 UV ;
in vec3 norm ;
in vec4 fragPosClipSpace;
in vec3 posInWS ;
in mat3 TBN;

///////////////////////// UNIFORMS

uniform float u_ambientFactor;
uniform float u_metallic;
uniform float u_roughness;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D roughTexture;
uniform sampler2D metalTexture;
uniform sampler2D aoTexture;
//uniform sampler2D depthMap;

///////////////////////// FUNCTIONS

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0);
float GeometrySchlickGGX(float NdotV);
float DistributionGGX();
float GeometrySmith();
//bool doesPassDepthTest();
vec3 getPointLight(int idx);

///////////////////////// Globals

const float PI = 3.14159265 ;
float NdotL ;
float NdotV ;
float NdotH ;
float VdotH ;

float alpha ;
float metal ;
float AO ;
vec3 F0;
vec3 alb;
vec3 normal;

///////////////////////// MAIN

void main()
{    
     //if (!doesPassDepthTest()) return;

    vec2 uv = UV ; 
    vec3 N = texture(normalTexture, uv).rgb;
    N = normalize(TBN * (N * 2.0 - 1.0));
   
    vec3 alb = texture(albedoTexture, uv).rgb;
    alpha = texture(roughTexture, uv).r;
    metal = texture(metalTexture, uv).r  ;   

    AO = texture(aoTexture, uv).r;
    vec3 F0 = mix(vec3(0.04), alb, metal); 


    vec3 V = normalize(u_viewPos - posInWS);    // View Direction
    vec3 L = normalize(-dLight.direction);      // Light Direction
    vec3 H = normalize(L + V);                // Half-way vector  

    NdotL = max(dot(N, L), 0.0);              // cache these calculations
    NdotV = max(dot(N, V), 0.0);
    NdotH = max(dot(N, H), 0.0);

    /////// Cook Torrence Specular Term

    /////////////////////
    // Cook-Torrance BRDF
    float D = DistributionGGX();   
    float G = GeometrySchlickGGX(NdotV) * GeometrySchlickGGX(NdotL) ; 
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
    vec3 numerator    = D * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;	  
    vec3 Lo = (kD * alb / PI + specular) * NdotL;  
    vec3 ambient = vec3(0.03) * alb * AO;
    
    vec3 color = ambient + Lo;

    for (int i = 0; i < numPointLights; i++) {
        color += getPointLight(i);
    }

    FragColour = vec4(color, 1.0); 
}

vec3 getPointLight(int idx)
{
	// vec3 ambient = ambientStrength * pLights[idx].colour;
	vec3 ambient = u_ambientFactor * vec3(1.0) * alb;
	float distance = length(pLights[idx].position - posInWS);
	float attn = 1/(pLights[idx].constants.x + (pLights[idx].constants.y* distance) + (pLights[idx].constants.z * (distance * distance)));
	vec3 lightDir = normalize(pLights[idx].position - posInWS);

	vec3 V = normalize(u_viewPos - posInWS);    // View Direction
    vec3 L = normalize(-dLight.direction);      // Light Direction
    vec3 H = normalize(L + V);                // Half-way vector

	NdotL = max(dot(normal, L), 0.0001);
	NdotH = max(dot(normal, H), 0.0001);
	NdotV = max(dot(normal, V), 0.0001);
	VdotH = max(dot(V, H), 0.0001);

    float D = DistributionGGX();
	float G = GeometrySchlickGGX(NdotV) * GeometrySchlickGGX(NdotL) ; 
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metal;

	vec3 diff = (kD * alb / PI) * NdotL;
	vec3 diffuse = diff * attn * pLights[idx].colour;
	vec3 viewDir = normalize(u_viewPos - posInWS);
	vec3 reflectDir = reflect(-lightDir, normal); 
	
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

float DistributionGGX()
{
    float a = alpha * alpha;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return a2 / denominator;
}

float GeometrySchlickGGX(float Ndot)
{
    float r = (alpha + 1.0);
    float k = (r * r) / 8.0;
    float denominator = Ndot * (1.0 - k) + k;

    return Ndot / denominator;
}

float GeometrySmith()
{
    float ggx2 = GeometrySchlickGGX(NdotV);
    float ggx1 = GeometrySchlickGGX(NdotL);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0)
{
    return F0 + (max(vec3(1.0 - alpha), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

//bool doesPassDepthTest() {
//    float fragDepth = fragPosClipSpace.z / fragPosClipSpace.w;
//    fragDepth = fragDepth * 0.5 + 0.5; 
//
//    vec2 fragCoord = (fragPosClipSpace.xy / fragPosClipSpace.w) * 0.5 + 0.5;
//    float depthPrePass = texture(depthMap, fragCoord).r;
//
//    float bias = 0.001;
//    if (fragDepth >= depthPrePass + bias) return false;  
//    return true;
//}