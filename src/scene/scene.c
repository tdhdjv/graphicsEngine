#ifndef SCENE_HEADER
#define SCENE_HEADER

/*
 * This File is incharge or defining datatypes
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <stb_image.h>

#include <stdint.h>
#include "cglm/affine-pre.h"
#include "mesh.c"
#include "../data_types/io.c"
#include "../data_types/pool.c"

/*
GLuint create_texture(const char* filename) {
  //texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  unsigned char* texData = stbi_load(filename, &width, &height, &nrChannels, 0);
  if(texData) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else {
    printf("Failed to load Image");
  }
  stbi_image_free(texData);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}
*/

GLuint create_shaders(Arena* arena) {
  int success;
  char infoLog[512];

  GLuint program;
  program = glCreateProgram();

  ArenaMark mark = create_arena_mark(arena);

  // Vertex Shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  String vertexSource = read_file(arena, "res/vertex.glsl");
  CSTRING(vertexSource, CvertexSource);
  glShaderSource(vertexShader, 1, &CvertexSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("Failed to compile Vertex Shader\n");
    printf("%s\n", infoLog);
  }
  glAttachShader(program, vertexShader);
  glDeleteShader(vertexShader);

  arena_return_to_mark(&mark);
  
  mark = create_arena_mark(arena);

  // FragmentShader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  String fragmentSource = read_file(arena, "res/fragment.glsl");
  CSTRING(fragmentSource, CfragmentSource);
  glShaderSource(fragmentShader, 1, &CfragmentSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    printf("Failed to compile Fragment Shader\n");
    printf("%s\n", infoLog);
  }
  glAttachShader(program, fragmentShader);
  glDeleteShader(fragmentShader);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  arena_return_to_mark(&mark);

  return program;
}

void render_scene(Scene* scene, ShaderProgram shaderProgram) {
  mat4 projectionMatrix;
  glm_perspective(glm_rad(90), 1000.0/800.0, 0.1, 100.0, projectionMatrix);

  //calculate the viewMatrix
  mat4 viewMatrix;
  vec3 origin;
  vec3 up = {0.0f, 1.0f, 0.0f};
  glm_vec3_add(scene->camera.facing, scene->camera.position, origin);
  glm_lookat(scene->camera.position, origin, up, viewMatrix);

  for(size_t i = 0; i < scene->modelList.capacity; i++) {
    Model model = array_get(Model, &scene->modelList, i);
    Array(Mesh) meshList = model.meshList;
    mat4 modelMatrix;
    glm_mat4_copy(model.modelMatrix, modelMatrix);
    for(size_t j = 0; j < meshList.capacity; j++) {
      Mesh mesh = array_get(Mesh, &meshList, j);
      glUseProgram(shaderProgram);
      int32_t modelLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
      int32_t viewLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
      int32_t projectionLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");

      if(modelLocation != -1) glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (GLfloat*)modelMatrix);
      if(viewLocation != -1) glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (GLfloat*)viewMatrix);
      if(projectionLocation != -1) glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (GLfloat*)projectionMatrix);
      
      glBindVertexArray(mesh.geometry.vao);
      glDrawElements(GL_TRIANGLES, mesh.geometry.indexCount, GL_UNSIGNED_INT, 0);
    }
  }
}

#endif
