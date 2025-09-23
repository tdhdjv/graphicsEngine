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
#include "gltf_loader.c"
#include "bloom.c"

GLFWwindow* window;
static int windowWidth, windowHeight;

GLuint fbo, rbo;
Texture frameTexture, threshHoldTexture;
GLuint gBufferFbo, gBufferRbo;
Texture positionBuffer, normalBuffer, albedoBuffer, metallicRoughnessBuffer, emissiveBuffer;

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
  //for debug perpose only
  if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    printf("%f, %f, %f\n", cam->position[0], cam->position[1], cam->position[2]);
    fflush(stdout);
  }

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

  glBindTexture(GL_TEXTURE_2D, threshHoldTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
}

void render_g_buffer(const Scene* scene) {

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
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  // Create a windowed mode window and its OpenGL context
  windowWidth = width, windowHeight = height;
  window = glfwCreateWindow(windowWidth, windowHeight, "My Graphics Engine!", NULL, NULL); 

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

void setup_framebuffer(void) {
  //render framebuffer setup
  glGenFramebuffers(1, &fbo);
  glGenRenderbuffers(1, &rbo);
  glGenTextures(1, &frameTexture);
  glGenTextures(1, &threshHoldTexture);

  glBindTexture(GL_TEXTURE_2D, frameTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, threshHoldTexture);
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
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, threshHoldTexture, 0);
  const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, buffers);
}

void setup_gBuffer(void) {
  //g buffer framebuffer setup
  glGenFramebuffers(1, &gBufferFbo);
  glGenRenderbuffers(1, &gBufferRbo);
  glGenTextures(1, &frameTexture);
  glGenTextures(1, &threshHoldTexture);

  glBindTexture(GL_TEXTURE_2D, positionBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, normalBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D,albedoBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, normalBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, metallicRoughnessBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, emissiveBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindRenderbuffer(GL_RENDERBUFFER, gBufferRbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);

  glBindFramebuffer(GL_FRAMEBUFFER, gBufferFbo);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gBufferRbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, threshHoldTexture, 0);
  const GLenum gBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
  glDrawBuffers(5, gBuffers);
}

int main(void) {
  init_window(960, 540);

  char* data = malloc(1<<27);
  Arena arena = create_arena(data, 1<<27);

  //setup
  setup_environment_map();
  setup_render(&arena);
  setup_material(&arena);
  setup_framebuffer();
  //setup_gBuffer();
  Texture bloomBackBufferTexture;
  PostProcess bloom = setup_bloom(&arena, threshHoldTexture, &bloomBackBufferTexture);

  ShaderProgram bloomTextureShader = create_shader_program();
  attach_shader_to_program(&arena, &bloomTextureShader, GL_VERTEX_SHADER, create_string_from_literal("res/shader/quadVertex.glsl"));
  attach_shader_to_program(&arena, &bloomTextureShader, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/upsample.glsl"));
  finalize_shader_program(&bloomTextureShader);

  Material bloomTextureMaterial = create_material(&arena, &bloomTextureShader);
  material_set_texture(&bloomTextureMaterial, create_string_from_literal("screenTexture"), frameTexture);
  material_set_texture(&bloomTextureMaterial, create_string_from_literal("bloomTexture"), bloomBackBufferTexture);
  
  //scene descriptions
  ShaderProgram skyboxShader = create_shader_program();
  attach_shader_to_program(&arena, &skyboxShader, GL_VERTEX_SHADER, create_string_from_literal("res/shader/skyboxVertex.glsl"));
  attach_shader_to_program(&arena, &skyboxShader, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/skyboxFragment.glsl"));
  finalize_shader_program(&skyboxShader);
  glUseProgram(skyboxShader.id);

  Material skyBoxMaterial = create_material(&arena, &skyboxShader);
  //Texture environmentMap = create_environment_map("res/black.jpg", "res/black.jpg", "res/black.jpg", "res/black.jpg", "res/black.jpg", "res/black.jpg");//
  Texture environmentMap = create_environment_map("res/skybox/right.jpg", "res/skybox/left.jpg", "res/skybox/top.jpg", "res/skybox/bottom.jpg", "res/skybox/front.jpg", "res/skybox/back.jpg");

  setup_pbr(&arena, environmentMap);
  material_set_texture(&skyBoxMaterial, create_string_from_literal("environmentMap"), environmentMap);

  Array(Mesh) meshes = load_gltf(&arena, create_string_from_literal("res/gltf/Lantern/"), create_string_from_literal("Lantern.gltf"));

  Camera camera = {{0,0,0}, {0, 0, 1}};
  DynamicArray(OmniDirectionalLight) omniLights = create_dynamic_array(OmniDirectionalLight, 1);
  DynamicArray(DirectionalLight) directionalLights = create_dynamic_array(DirectionalLight, 1);
  vec3 temp1 = {9.0, 17.0, 0.0};
  vec3 temp2 = {1000.0,1000.0, 1000.0};
  vec3 temp3 = {0.0, 1.0, 1.0};
  vec3 temp4 = {10.0,10.0, 10.0};
  
  OmniDirectionalLight light1;
  light1.position[0] = temp1[0];
  light1.position[1] = temp1[1];
  light1.position[2] = temp1[2];

  light1.color[0] = temp2[0];
  light1.color[1] = temp2[1];
  light1.color[2] = temp2[2];

  DirectionalLight light2;
  light2.direction[0] = temp3[0];
  light2.direction[1] = temp3[1];
  light2.direction[2] = temp3[2];

  light2.color[0] = temp4[0];
  light2.color[1] = temp4[1];
  light2.color[2] = temp4[2];

  dynamic_array_append(OmniDirectionalLight, &omniLights, &light1);
  dynamic_array_append(DirectionalLight, &directionalLights, &light2);

  Scene scene = (Scene){meshes, directionalLights, omniLights, {0}, camera, skyBoxMaterial};
  
  /* renders */

  double previousTime = 0;
  while (!glfwWindowShouldClose(window)) {
    double currentTime = glfwGetTime();
    float dt = (float)(currentTime - previousTime);
    previousTime = currentTime;

    input(window, &(scene.camera), dt);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    render_scene(&scene, windowWidth, windowHeight);
    
    //rendering the texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    post_process(&bloom.materials, &bloom.dispatchSizes);

    glUseProgram(bloomTextureMaterial.shaderProgram->id);
    material_push_uniform_values(&bloomTextureMaterial);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(quadVAO);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  glfwTerminate();
  free_arena(&arena);

  return 0;
}
