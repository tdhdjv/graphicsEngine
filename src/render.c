#ifndef SCENE_HEADER
#define SCENE_HEADER

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image.h>

#include <stdint.h>

#include "data_types/array.c"
#include "data_types/string.c"
#include "scene_define.c"
#include "shader.c"

static GLuint quadVAO;
static Material quadMaterial;
static ShaderProgram quadShader;

static mat4 projectionMatrix;
/*
void render_scene(Scene* scene, u16 windowWidth, u16 windowHeight) {
  glViewport(0, 0, windowWidth, windowHeight);
  SceneNode* root = scene->root;
  for(root.)
}
*/

#endif
