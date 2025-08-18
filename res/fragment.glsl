#version 330 core

out vec4 outColor;

in vec3 Normal;
//in vec2 UV;

//uniform float time;
//uniform sampler2D tex;

void main() {
  vec3 normal = normalize(Normal);
  outColor = vec4(vec3(max(dot(Normal, normalize(vec3(-0.2, 1.0, -0.5))), 0.0)), 1.0);//texture(tex, UV);
}
