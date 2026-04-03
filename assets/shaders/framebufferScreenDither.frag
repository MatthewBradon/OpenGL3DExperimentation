#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;

float bayer4x4(vec2 position) {
    // Use integer pixel coordinates so the dither pattern aligns to pixels
    ivec2 p = ivec2(floor(position)) % 4;
    int index = p.x + p.y * 4;

    float bayer[16] = float[](
        0.0,  8.0,  2.0, 10.0,
        12.0, 4.0, 14.0,  6.0,
        3.0, 11.0, 1.0,  9.0,
        15.0, 7.0, 13.0,  5.0
    );

    return bayer[index] / 16.0;
}

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    float grayscaleColor = dot(color, vec3(0.299, 0.587, 0.114));

    grayscaleColor = 1.0 - grayscaleColor;

    float threshold = bayer4x4(gl_FragCoord.xy * 0.2);

    float levels = 8.0;
    float dithered = floor(grayscaleColor * levels + threshold) / levels;

    

    // float dithered = grayscaleColor < threshold ? 0.0 : 1.0;

    FragColor = vec4(vec3(dithered), 1.0);
}