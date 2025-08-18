#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cglm/cam.h>
#include <cglm/cglm.h>
#include <cglm/vec3.h>
#include <stb_image.h>

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "cglm/mat4.h"
#include "data_types/arena.c"
#include "data_types/pool.c"
#include "scene/mesh.c"
#include "resource/gltf_loader.c"
#include "scene/scene.c"

void input(GLFWwindow *window, Camera *cam, float dt) {
  vec3 forward, left, back, right;
  vec3 up = {0.0f, 1.0f, 0.f};
  vec3 down = {0.0f, -1.0f, 0.f};

  glm_vec3_cross(cam->facing, up, right);
  glm_vec3_normalize(right);
  glm_vec3_negate_to(right, left);

  glm_vec3_cross(up, right, forward);
  glm_vec3_negate_to(forward, back);

  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    glm_vec3_rotate(cam->facing, dt, up);
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    glm_vec3_rotate(cam->facing, -dt, up);

  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && cam->facing[1] > -0.99f)
    glm_vec3_rotate(cam->facing, -dt, right);
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && cam->facing[1] < 0.99f)
    glm_vec3_rotate(cam->facing, dt, right);

  glm_vec3_scale(forward, dt, forward);
  glm_vec3_scale(back, dt, back);
  glm_vec3_scale(left, dt, left);
  glm_vec3_scale(right, dt, right);
  glm_vec3_scale(up, dt, up);
  glm_vec3_scale(down, dt, down);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    glm_vec3_add(cam->position, forward, cam->position);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    glm_vec3_add(cam->position, left, cam->position);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    glm_vec3_add(cam->position, back, cam->position);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    glm_vec3_add(cam->position, right, cam->position);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    glm_vec3_add(cam->position, up, cam->position);
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    glm_vec3_add(cam->position, down, cam->position);

  glm_vec3_normalize(cam->facing);
}

GLFWwindow *create_window() {
  GLFWwindow *window;

  if (!glfwInit())
    return NULL;

  glfwWindowHint(GL_MAJOR_VERSION, 4);
  glfwWindowHint(GL_MAJOR_VERSION, 3);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

  // Create a windowed mode window and its OpenGL context
  window = glfwCreateWindow(1000, 800, "Hello World", NULL, NULL);

  if (!window) {
    glfwTerminate();
    printf("Failed to create GLFW Window\n");
    return NULL;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return NULL;
  }
  /*
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  */
  glViewport(0, 0, 1000, 800);//mode->width, mode->height);
  return window;
}

void GLAPIENTRY
  MessageCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf(stderr, "OpenGL Error: %s\n", message);
  fflush(stderr);
}

DEFINE_POOL(Mesh)
int main(void) {
  GLFWwindow *window = create_window();
  if (window == NULL)
    return -1;

  // OPENGL SHIT
  //glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallBack, 0);

  char* data = malloc(1<<24);
  Arena arena = create_arena(data, 1<<24);

  // pool
  Camera camera = {{0,0,0}, {0, 0, 1}};
  Model* modelArrayData = arena_alloc_array(&arena, Model, 1);
  Mesh* meshArrayData = arena_alloc_array(&arena, Mesh, 1);
  Array(Mesh) meshes = create_array(Mesh, meshArrayData, 1);
  Array(Model) models = create_array(Model, modelArrayData, 1); 
  //vec3 up = {0, 1, 0}, right = {1, 0, 0}; 
  meshes.data[0].geometry = extract_geometry_from_gltf(&arena, STRING("res/cube.gltf"));//generate_icosphere_geometry(&arena, 32);
  models.data[0].meshList = meshes;
  glm_mat4_copy(GLM_MAT4_IDENTITY,models.data[0].modelMatrix);
  vec3 translate = {0, 0, 5};
  glm_translate(models.data[0].modelMatrix, translate);

  Scene scene = (Scene){models, camera};

  double previousTime = 0;
  ShaderProgram shaderProgram = create_shaders(&arena);

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    // updates
    double currentTime = glfwGetTime();
    float dt = (float)(currentTime - previousTime);
    previousTime = currentTime;

    input(window, &(scene.camera), dt);
    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_scene(&scene, shaderProgram);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  

  glfwTerminate();
  free_arena(&arena);

  return 0;
}
