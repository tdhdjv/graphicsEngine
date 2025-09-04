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

#include "data_types/arena.c"
#include "pbr.c"
#include "data_types/array.c"
#include "data_types/string.c"
#include "scene_define.c"
#include "shader.c"
#include "material.c"
#include "environment_map.c"
#include "render.c"
#include "mesh.c"
#include "post_process.c"

GLFWwindow* window;
static int windowWidth, windowHeight;

GLuint fbo, rbo;
Texture frameTexture;

void input(GLFWwindow *window, Camera *cam, float dt) {
  float speed = 5.0;
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

  glm_vec3_scale(forward, speed*dt, forward);
  glm_vec3_scale(back, speed*dt, back);
  glm_vec3_scale(left, speed*dt, left);
  glm_vec3_scale(right, speed*dt, right);
  glm_vec3_scale(up, speed*dt, up);
  glm_vec3_scale(down, speed*dt, down);

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

void GLAPIENTRY message_call_back(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf(stderr, "OpenGL Error: %s\n", message);
  fflush(stderr);
}

void window_resize_call_back(GLFWwindow* window, int width, int height) {
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
  glBindTexture(GL_TEXTURE_2D, frameTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
}

void init_window(uint16_t width, uint16_t height) {

  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize glfw\n");
    fflush(stderr);
    abort();
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  // Create a windowed mode window and its OpenGL context
  windowWidth = width, windowHeight = height;
  window = glfwCreateWindow(windowWidth, windowHeight, "Hello World", NULL, NULL); 

  if (!window) {
    glfwTerminate();
    fprintf(stderr, "Failed to create window\n");
    fflush(stderr);
    abort();
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initilize GLAD\n");
    fflush(stderr);
    abort();
  }
  glViewport(0, 0, 1000, 800);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glEnable(GL_DEBUG_OUTPUT);

  glDebugMessageCallback(message_call_back, 0);
  glfwSetWindowSizeCallback(window, window_resize_call_back);
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
}

int main(void) {

  init_window(1000, 800);

  char* data = malloc(1<<27);
  Arena arena = create_arena(data, 1<<27);

  //setup
  setup_environment_map();
  setup_render(&arena);
  setup_material(&arena);
  setup_post_process(windowWidth, windowHeight);


  // framebuffer setup
  glGenFramebuffers(1, &fbo);
  glGenRenderbuffers(1, &rbo);
  glGenTextures(1, &frameTexture);

  glBindTexture(GL_TEXTURE_2D, frameTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameTexture, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "'fbo' is not complete!");
    fflush(stderr);
    abort();
  }
  
  //scene descriptions
  ShaderProgram skyboxShader = create_shader_program();
  attach_shader_to_program(&arena, &skyboxShader, GL_VERTEX_SHADER, create_string_from_literal("res/shader/skyboxVertex.glsl"));
  attach_shader_to_program(&arena, &skyboxShader, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/skyboxFragment.glsl"));
  finalize_shader_program(&skyboxShader);
  glUseProgram(skyboxShader.id);

  Material skyBoxMaterial = create_material(&arena, &skyboxShader);
  Texture environmentMap = create_environment_map("res/skybox/right.jpg", "res/skybox/left.jpg", "res/skybox/top.jpg", "res/skybox/bottom.jpg", "res/skybox/front.jpg", "res/skybox/back.jpg");
  //Texture test = create_texture("res/skybox/right.jpg");

  setup_pbr(&arena, environmentMap);
  material_set_texture(&skyBoxMaterial, create_string_from_literal("environmentMap"), environmentMap);

  Mesh* meshArrayData = arena_alloc_array(&arena, Mesh, 64);
  Array(Mesh) meshes =  create_array(Mesh, meshArrayData, 64);
  vec3 albedo = {1.0, 0.0, 0.0};
  vec3 emissive = {0.0, 0.0, 0.0};

  //mesh setup
  for(int i = 0; i < 8; i++) { 
    for(int j = 0; j < 8; j++) {
      Mesh mesh = {0};
      mesh.renderData = generate_icosphere(&arena, 16);
      
      mesh.material = create_pbr_material_values(&arena, albedo, ((float)i)/7.0f, ((float)j)/7.0f, emissive);
      mat4 modelMatrix = GLM_MAT4_IDENTITY_INIT;
      vec3 translation = {-j*2.2, i*2.2 , 10.0};
      glm_translate(modelMatrix, translation);
      glm_mat4_copy(modelMatrix, mesh.modelMatrix);
      *array_index(Mesh, &meshes, i*8 + j) = mesh;
    }
  }

  Camera camera = {{0,0,0}, {0, 0, 1}};

  Scene scene = (Scene){meshes, camera, skyBoxMaterial};
  
  // Post process
  ShaderProgram bloomShaderProgram = create_shader_program();
  attach_shader_to_program(&arena, &bloomShaderProgram, GL_COMPUTE_SHADER, create_string_from_literal("res/shader/bloom.glsl"));
  finalize_shader_program(&bloomShaderProgram);
  Material bloomMaterial = create_material(&arena, &bloomShaderProgram);

  DynamicArray(Material) postProcessList = create_dynamic_array(Material, 1);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial);

  /* renders */

  double previousTime = 0;
  while (!glfwWindowShouldClose(window)) {
    double currentTime = glfwGetTime();
    float dt = (float)(currentTime - previousTime);
    previousTime = currentTime;

    input(window, &(scene.camera), dt);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    render_scene(&scene, windowWidth, windowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Texture outputTexture = post_process(&postProcessList, frameTexture, windowWidth, windowHeight);
    render_texture(outputTexture);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  glfwTerminate();
  free_arena(&arena);

  return 0;
}
