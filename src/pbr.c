#ifndef PBR_IMPL
#define PBR_IMPL

#include <glad/glad.h>
#include <cglm/cglm.h>

void create_renderProbe() {
  unsigned int prefilterMap;
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  for(unsigned i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, NULL);
  }
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  vec3 origin = {0.0, 0.0, 0.0};
  vec3 up = {0.0, 1.0, 0.0};
  vec3 down = {0.0, -1.0, 0.0};
  vec3 front = {0.0, 0.0, 1.0};
  vec3 back = {0.0, 0.0, -1.0};
  vec3 right = {1.0, 0.0, 0.0};
  vec3 left = {-1.0, 0.0, 0.0};

  mat4 captureViews[6];
  glm_lookat(origin, right, down, captureViews[0]);
  glm_lookat(origin, left, down, captureViews[1]);
  glm_lookat(origin, up, front, captureViews[2]);
  glm_lookat(origin, down, back, captureViews[3]);
  glm_lookat(origin, front, down, captureViews[4]);
  glm_lookat(origin, back, down, captureViews[5]);
  mat4 projectionMatrix;

  glm_perspective(glm_rad(90.0f), 1.0f, 0.1f, 10.0f, projectionMatrix);

  GLuint envShaderProgram = create_shaders(&arena, "res/shader/skyBoxVertex.glsl", "res/shader/prefilter.glsl");
  glUseProgram(envShaderProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skyBox);

  int32_t skyBox = glGetUniformLocation(envShaderProgram, "environmentMap");
  int32_t roughness = glGetUniformLocation(envShaderProgram, "roughness");
  int32_t projectionLocation0 = glGetUniformLocation(envShaderProgram, "projection");
  int32_t viewLocation0 = glGetUniformLocation(envShaderProgram, "view"); 

  if(skyBox != -1) glUniform1i(skyBox, 0);
  if(projectionLocation0 != -1) glUniformMatrix4fv(projectionLocation0, 1, GL_FALSE, (GLfloat*)projectionMatrix);

  const unsigned maxMipLevels = 5;

  for(unsigned mip = 0; mip < maxMipLevels; ++mip) {

    unsigned int mipWidth  = 128 >> mip;
    unsigned int mipHeight = 128 >> mip;

    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

    glViewport(0, 0, mipWidth, mipHeight);
    float roughnessValue = ((float)mip)/((float)(maxMipLevels - 1));
    if(roughness != -1) glUniform1f(roughness, roughnessValue);

    for(unsigned i = 0; i < 6; i++) {
      glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
      if(viewLocation0 != -1) glUniformMatrix4fv(viewLocation0, 1, GL_FALSE, (GLfloat*)captureViews[i]);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindVertexArray(cubeMapVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  }

}

#endif
