#version 460 core

#extension GL_ARB_bindless_texture : require

out vec4 colour;

in vec3 texCoords;

layout(bindless_sampler) uniform samplerCube u_skybox;

void main()
{    
    colour = texture(u_skybox, texCoords);
}
