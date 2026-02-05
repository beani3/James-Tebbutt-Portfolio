#version 460 core
			
layout(location = 0) in vec3 a_vertexPosition;
layout(location = 1) in vec3 a_vertexNormal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec3 a_tangent;

out vec3 fragmentPos;
out vec2 texCoord;
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
	vec3 T = (u_model * vec4(a_tangent, 0.0)).xyz;
	T = normalize(T - dot(T, a_vertexNormal) * a_vertexNormal);
	vec3 B = cross(a_vertexNormal, T);
	B = normalize(B);
	TBN = mat3(T, B, a_vertexNormal);

	fragmentPos = vec3(u_model * vec4(a_vertexPosition, 1.0));

	texCoord = a_texCoord;
	gl_Position = u_projection * u_view * vec4(fragmentPos,1.0);
}