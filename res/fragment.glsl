#version 330 core

out vec4 outColor;

in vec3 Normal;

uniform float time;

void main() {
  vec3 normal = normalize(Normal);
  outColor = vec4(normal, 1.0);
}
