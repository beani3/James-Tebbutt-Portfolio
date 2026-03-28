#version 460 core
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aUV;
layout(location = 3) in vec3 aTan;

out vec3 norm;
out vec3 posInWS ;
out vec4 fragPosClipSpace;
out vec2 UV ;
out mat3 TBN;

layout (std140, binding = 0) uniform b_camera
{
	uniform mat4 u_view;
	uniform mat4 u_projection;
	uniform vec3 u_viewPos;
};

uniform mat4 u_model;

void main()
{  
    posInWS = (u_model*vec4(aPos,1.0)).xyz; 
    vec3 fragmentPos = vec3(u_model * vec4(aPos, 1.0));
    fragPosClipSpace = u_projection * u_view * vec4(fragmentPos, 1.0);
    gl_Position = fragPosClipSpace;
    UV = aUV ;
    norm = (u_model*vec4(aNorm,0.0)).xyz;
    vec3 T = (u_model * vec4(aTan, 0.0)).xyz;
    vec3 B = cross(norm, T);
    B = normalize(B);
    TBN = mat3(T, B, norm);
}