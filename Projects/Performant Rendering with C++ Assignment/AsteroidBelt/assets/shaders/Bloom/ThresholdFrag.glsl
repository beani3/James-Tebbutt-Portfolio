#version 450 core
			
layout(location = 0) out vec4 colour;

in vec2 texCoord;

uniform sampler2D u_albedoMap;
uniform float u_threshold;

void main()
{
	vec3 rgb = texture(u_albedoMap, texCoord).rgb;
	float brightness = dot(rgb, vec3(0.2126, 0.7152, 0.0722));

	if(brightness > u_threshold) {
		colour = vec4(rgb,1);
	} else {
		colour = vec4(0.0, 0.0, 0.0, 1.0);
	}

}