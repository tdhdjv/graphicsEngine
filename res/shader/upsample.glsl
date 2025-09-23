#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;

void main()
{
    vec3 color = vec3(0.0); 
    color += textureLod(bloomTexture, TexCoords, 0).rgb;
    color += textureLod(bloomTexture, TexCoords, 1).rgb;
    color += textureLod(bloomTexture, TexCoords, 2).rgb;
    color += textureLod(bloomTexture, TexCoords, 3).rgb;
    color += textureLod(bloomTexture, TexCoords, 4).rgb;

    color += texture(screenTexture, TexCoords).rgb;

    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
} 
