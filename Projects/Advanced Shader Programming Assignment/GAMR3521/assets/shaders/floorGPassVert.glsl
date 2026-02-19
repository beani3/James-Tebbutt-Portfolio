#version 460 core
			
layout(location = 0) in vec3 a_vertexPosition;
layout(location = 1) in vec2 a_texCoord;

out vec3 vs_fragmentPos;
out vec2 vs_texCoord;
out vec4 vs_fragmentPosLightSpace;

layout (std140, binding = 0) uniform b_camera
{
	uniform mat4 u_view;
	uniform mat4 u_projection;
	uniform vec3 u_viewPos;
};

uniform mat4 u_model;
uniform mat4 u_lightSpaceTransform;

void main()
{
	vs_fragmentPos = vec3(u_model * vec4(a_vertexPosition, 1.0));
	vs_fragmentPosLightSpace = u_lightSpaceTransform * vec4(vs_fragmentPos, 1.0);

	vs_texCoord = a_texCoord ;
}