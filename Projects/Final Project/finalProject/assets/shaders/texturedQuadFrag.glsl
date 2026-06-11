#version 460 core 

#extension GL_ARB_bindless_texture : require

layout (location = 0) out vec4 fragColor;

layout (std430, binding  = 1) readonly buffer handlesSSBO {
	sampler2D textureHandles[];
};

in vec4 aCol;
in vec2 aUV;
flat in int fsInstance;

void main() {
	fragColor = aCol * texture(textureHandles[fsInstance], aUV);
	//fragColor = aCol;
}