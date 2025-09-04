#ifndef SCENE_HEADER
#define SCENE_HEADER

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image.h>

#include <stdint.h>

#include "data_types/string.c"
#include "material.c"
#include "scene_define.c"
#include "environment_map.c"
#include "shader.c"

static GLuint quadVAO;
static Material quadMaterial;
static ShaderProgram quadShader;

static mat4 projectionMatrix;

void setup_render(Arena* arena) {
  float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
  };

  unsigned quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  quadShader = create_shader_program()
  attach_shader_to_program(arena, &quadShader, GL_VERTEX_SHADER, create_string_from_literal("res/shader/quadVertex.glsl"));
  attach_shader_to_program(arena, &quadShader, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/quadFragment.glsl"));
  finalize_shader_program(&quadShader);
  quadMaterial = create_material(arena, &quadShader);
}

void render_mesh(Mesh* mesh, const Camera* camera, mat4 viewMatrix) {
  glUseProgram(mesh->material.shaderProgram->id);
  material_set_vec3(&mesh->material, create_string_from_literal("camPos"), camera->position);
  material_set_mat4(&mesh->material, create_string_from_literal("viewMatrix"), viewMatrix);
  material_set_mat4(&mesh->material, create_string_from_literal("projectionMatrix"), projectionMatrix);
  material_set_mat4(&mesh->material, create_string_from_literal("modelMatrix"), mesh->modelMatrix);
  material_push_uniform_values(&mesh->material);
  glBindVertexArray(mesh->renderData.vao);
  glDrawElements(GL_TRIANGLES, mesh->renderData.indexCount, GL_UNSIGNED_INT, 0);
}

void render_texture(Texture texture) {
  glUseProgram(quadMaterial.shaderProgram->id);
  material_set_texture(&quadMaterial, create_string_from_literal("screenTexture"), texture);
  material_push_uniform_values(&quadMaterial);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(quadVAO);
  glDisable(GL_CULL_FACE);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_scene(Scene* scene, int windowWidth, int windowHeight) {
  //perspective matrix
  glViewport(0, 0, windowWidth, windowHeight);
  glm_perspective(glm_rad(90.0f), (float)windowWidth/(float)windowHeight, 0.1f, 100.0f, projectionMatrix);

  //camera matrices
  mat4 viewMatrix, skyboxViewMatrix;
  vec3 center, facing, position;
  vec3 up = {0.0f, 1.0f, 0.0f}, origin = {0.0f, 0.0f, 0.0f};

  facing[0] = scene->camera.facing[0];
  facing[1] = scene->camera.facing[1];
  facing[2] = scene->camera.facing[2];
  
  position[0] = scene->camera.position[0];
  position[1] = scene->camera.position[1];
  position[2] = scene->camera.position[2];

  glm_vec3_add(facing, position, center);
  glm_lookat(position, center, up, viewMatrix);
  glm_lookat(origin, facing, up,skyboxViewMatrix);

  //render skybox
  glDisable(GL_CULL_FACE);
  glUseProgram(scene->skyBoxMaterial.shaderProgram->id);

  material_set_mat4(&scene->skyBoxMaterial, create_string_from_literal("viewMatrix"), skyboxViewMatrix);
  material_set_mat4(&scene->skyBoxMaterial, create_string_from_literal("projectionMatrix"), projectionMatrix);
  material_push_uniform_values(&scene->skyBoxMaterial);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(cubeMapVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glEnable(GL_CULL_FACE);
  glClear(GL_DEPTH_BUFFER_BIT);

  //render meshes
  for(size_t i = 0; i < scene->meshList.length; i++) {
    render_mesh(&scene->meshList.data[i], &scene->camera, viewMatrix);
  }
}

#endif
