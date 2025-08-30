#ifndef ENVIRONMENT_MAP_IMPL
#define ENVIRONMENT_MAP_IMPL

#include <glad/glad.h>
#include <stb_image.h>
#include "data_types/string.c"
#include "scene_define.c"
#include "material.c"

static GLuint captureFbo;
//static GLuint captureRbo;
static mat4 captureViews[6];
static mat4 captureProjection;

static GLuint cubeMapVAO;

//Sets up the values and objects used for environment mapping
void setup_environment_map(void) { 
  glGenFramebuffers(1, &captureFbo);
  //view matrices
  vec3 origin = {0.0, 0.0, 0.0};
  vec3 up = {0.0, 1.0, 0.0};
  vec3 down = {0.0, -1.0, 0.0};
  vec3 front = {0.0, 0.0, 1.0};
  vec3 back = {0.0, 0.0, -1.0};
  vec3 right = {1.0, 0.0, 0.0};
  vec3 left = {-1.0, 0.0, 0.0};
  glm_lookat(origin, right, down, captureViews[0]);
  glm_lookat(origin, left, down, captureViews[1]);
  glm_lookat(origin, up, front, captureViews[2]);
  glm_lookat(origin, down, back, captureViews[3]);
  glm_lookat(origin, front, down, captureViews[4]);
  glm_lookat(origin, back, down, captureViews[5]);

  //perspective matrix
  glm_perspective(glm_rad(90.0f), 1.0f, 0.1f, 10.0f, captureProjection);

  //cubeMap
  float cubeMapVertices[] = {
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
       0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
      -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
      -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
      -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
       0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
       0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
       0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
  };

  unsigned cubeMapVBO;
  glGenVertexArrays(1, &cubeMapVAO);
  glGenBuffers(1, &cubeMapVBO);
  glBindVertexArray(cubeMapVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeMapVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeMapVertices), &cubeMapVertices, GL_STATIC_DRAW);
  
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
}

GLuint create_environment_map(const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
  int width, height, nrChannels;
  unsigned char *data;  

  data = stbi_load(right, &width, &height, &nrChannels, 4);
  glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  data = stbi_load(left, &width, &height, &nrChannels, 4);
  glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  data = stbi_load(top, &width, &height, &nrChannels, 4);
  glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  data = stbi_load(bottom, &width, &height, &nrChannels, 4);
  glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  data = stbi_load(front, &width, &height, &nrChannels, 4);
  glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  data = stbi_load(back, &width, &height, &nrChannels, 4);
  glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    
  return textureID;
}

GLuint create_reflection_probe_env(uint16_t length, Texture envMap, Material* captureMaterial) {
  GLuint probeMap;
  glGenTextures(1, &probeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, probeMap);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  for(unsigned i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, length, length, 0, GL_RGB, GL_FLOAT, NULL);
  }
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "Framebuffer incomplete!");
    fflush(stderr);
    abort();
  }

  material_set_texture(captureMaterial, create_string_from_literal("evironmentMap"), envMap);
  material_set_mat4(captureMaterial, create_string_from_literal("projectionMatrix"), captureProjection);
  glUseProgram(captureMaterial->shaderProgram->id);
  glViewport(0, 0, length, length);

  for(unsigned i = 0; i < 6; i++) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, probeMap, 0);
    material_set_mat4(captureMaterial, create_string_from_literal("viewMatrix"), captureViews[i]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(cubeMapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);  
  }

  return probeMap;
}

GLuint create_reflection_probe_scene(uint16_t length, Scene* scene, Material captureMaterial, vec3 position) {
  return 0;
}
#endif
