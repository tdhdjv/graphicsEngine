#ifndef SCENE_HEADER
#define SCENE_HEADER

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image.h>

#include <stdint.h>

#include "data_types/array.c"
#include "resource.c"
#include "scene_define.c"

void render_scene(Scene* scene, ShaderProgram shaderProgram, ShaderProgram skyBoxShaderProgram, GLuint prefilterTexture, GLuint brdfLUTT) {
  Camera camera = scene->camera;
  mat4 projectionMatrix;
  glm_perspective(glm_rad(90), 1000.0/800.0, 0.1, 100.0, projectionMatrix);

  //calculate the viewMatrix
  mat4 viewMatrix;
  vec3 origin;
  vec3 up = {0.0f, 1.0f, 0.0f};
  glm_vec3_add(scene->camera.facing, scene->camera.position, origin);
  glm_lookat(scene->camera.position, origin, up, viewMatrix);
  mat4 skyBoxViewMatrix;
  glm_mat4_copy(viewMatrix, skyBoxViewMatrix);
  skyBoxViewMatrix[3][0] = 0.0;
  skyBoxViewMatrix[3][1] = 0.0;
  skyBoxViewMatrix[3][2] = 0.0;
  skyBoxViewMatrix[3][3] = 1.0;

  skyBoxViewMatrix[0][3] = 0.0;
  skyBoxViewMatrix[1][3] = 0.0;
  skyBoxViewMatrix[2][3] = 0.0;

  //skybox
  glUseProgram(skyBoxShaderProgram);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene->skyBox);

  int32_t skyBox = glGetUniformLocation(skyBoxShaderProgram, "skybox");
  int32_t projectionLocation0 = glGetUniformLocation(skyBoxShaderProgram, "projection");
  int32_t viewLocation0 = glGetUniformLocation(skyBoxShaderProgram, "view"); 

  if(skyBox != -1) glUniform1i(skyBox, 0);
  if(projectionLocation0 != -1) glUniformMatrix4fv(projectionLocation0, 1, GL_FALSE, (GLfloat*)projectionMatrix);
  if(viewLocation0 != -1) glUniformMatrix4fv(viewLocation0, 1, GL_FALSE, (GLfloat*)skyBoxViewMatrix);

  glBindVertexArray(cubeMapVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glClear(GL_DEPTH_BUFFER_BIT);

  Array(Mesh) meshList = scene->meshList; for(size_t j = 0; j < meshList.capacity; j++) { 
    Mesh mesh = array_get(Mesh, &meshList, j); 
    mat4 modelMatrix; glm_mat4_copy(mesh.modelMatrix, modelMatrix); 
    glUseProgram(shaderProgram); 
    int32_t modelLocation = glGetUniformLocation(shaderProgram, "model");
    int32_t viewLocation = glGetUniformLocation(shaderProgram, "view"); 
    int32_t projectionLocation = glGetUniformLocation(shaderProgram, "projection");

    int32_t albedoMap = glGetUniformLocation(shaderProgram, "albedoMap");
    int32_t normalMap = glGetUniformLocation(shaderProgram, "normalMap");
    int32_t roughnessMetallicMap = glGetUniformLocation(shaderProgram, "roughnessMetallicMap");
    int32_t aoMap = glGetUniformLocation(shaderProgram, "aoMap");
    int32_t emissiveMap = glGetUniformLocation(shaderProgram, "emissiveMap");

    int32_t albedoFactor = glGetUniformLocation(shaderProgram, "albedoFactor");
    int32_t metallicFactor = glGetUniformLocation(shaderProgram, "metallicFactor");
    int32_t roughnessFactor = glGetUniformLocation(shaderProgram, "roughnessFactor");
    int32_t emissiveFactor = glGetUniformLocation(shaderProgram, "emissiveFactor");

    int32_t prefilterMap = glGetUniformLocation(shaderProgram, "prefilterMap");
    int32_t brdfLUT = glGetUniformLocation(shaderProgram, "brdfLUT");
    
    int32_t camPos = glGetUniformLocation(shaderProgram, "camPos");

    if(camPos != -1) glUniform3f(camPos, camera.position[0], camera.position[1], camera.position[2]);
    if(modelLocation != -1) glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (GLfloat*)modelMatrix);
    if(viewLocation != -1) glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (GLfloat*)viewMatrix);
    if(projectionLocation != -1) glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (GLfloat*)projectionMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh.material.colorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh.material.normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mesh.material.metallicRoughnessTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mesh.material.occulsionTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mesh.material.emissiveTexture);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, brdfLUTT);

    if(albedoMap != -1) glUniform1i(albedoMap, 0);
    if(normalMap != -1) glUniform1i(normalMap, 1);
    if(roughnessMetallicMap != -1) glUniform1i(roughnessMetallicMap, 2);
    if(aoMap != -1) glUniform1i(aoMap, 3);
    if(emissiveMap != -1) glUniform1i(emissiveMap, 4);

    if(prefilterMap != -1) glUniform1i(prefilterMap, 5);
    if(brdfLUT != -1) glUniform1i(brdfLUT, 6);

    if(albedoFactor != -1) glUniform3f(albedoFactor, mesh.material.colorFactor[0], mesh.material.colorFactor[1], mesh.material.colorFactor[2]);
    if(metallicFactor != -1) glUniform1f(metallicFactor, mesh.material.metallicFactor);
    if(roughnessFactor != -1) glUniform1f(roughnessFactor, mesh.material.roughnessFactor);
    if(emissiveFactor != -1) glUniform3f(emissiveFactor, mesh.material.emissiveFactor[0], mesh.material.emissiveFactor[1], mesh.material.emissiveFactor[2]);

    
    glBindVertexArray(mesh.geometry.vao);
    glDrawElements(GL_TRIANGLES, mesh.geometry.indexCount, GL_UNSIGNED_INT, 0);
  }
}

#endif
