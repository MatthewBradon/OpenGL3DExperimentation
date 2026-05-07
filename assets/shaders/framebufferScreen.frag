#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform float exposure;
const float gamma = 2.2;


vec3 ReinhardToneMapping(vec3 hdrColor) {
	
	// exposure tone mapping
	vec3 ldrColor = vec3(1.0) - exp(-hdrColor * exposure);
	
	// gamma correction
	ldrColor = pow(ldrColor, vec3(1.0 / gamma));

	return ldrColor;
}

void main()
{
    vec4 color = texture(screenTexture, TexCoords);
    FragColor = vec4(ReinhardToneMapping(color.rgb), 1.0);
}

