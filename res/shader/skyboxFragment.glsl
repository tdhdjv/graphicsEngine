#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube environmentMap;

void main() {    
  vec3 color = texture(environmentMap, TexCoords).rgb;
  color = color/(vec3(1.001) - color);
  FragColor = vec4(color, 1.0);
}
