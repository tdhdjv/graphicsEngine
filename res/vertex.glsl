#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 Normal;

uniform float time;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
  mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
  gl_Position = MVP*vec4(position, 1.0);
  Normal = normal;
}
