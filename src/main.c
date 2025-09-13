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
#include "gltf_loader.c"

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
  //Texture environmentMap = create_environment_map("res/black.jpg", "res/black.jpg", "res/black.jpg", "res/black.jpg", "res/black.jpg", "res/black.jpg");//
  Texture environmentMap = create_environment_map("res/skybox/right.jpg", "res/skybox/left.jpg", "res/skybox/top.jpg", "res/skybox/bottom.jpg", "res/skybox/front.jpg", "res/skybox/back.jpg");

  setup_pbr(&arena, environmentMap);
  material_set_texture(&skyBoxMaterial, create_string_from_literal("environmentMap"), environmentMap);

  Mesh* meshArrayData = arena_alloc_array(&arena, Mesh, 1);
  Array(Mesh) meshes =  create_array(Mesh, meshArrayData, 1);
  *array_index(Mesh, &meshes, 0) = load_gltf(&arena);

  /*
  vec3 albedo = {1.0, 0.0, 0.0};
  vec3 emissive = {10.0, 0.1, 0.1};

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
  */

  Camera camera = {{0,0,0}, {0, 0, 1}};

  Scene scene = (Scene){meshes, camera, skyBoxMaterial};
  
  DynamicArray(PostProcess) postProcessList = create_dynamic_array(PostProcess, 2);

  // Post process
  ShaderProgram downSampleShaderProgram = create_shader_program();
  attach_shader_to_program(&arena, &downSampleShaderProgram, GL_COMPUTE_SHADER, create_string_from_literal("res/shader/bloom.glsl"));
  finalize_shader_program(&downSampleShaderProgram);
  Material downSampleMaterial = create_material(&arena, &downSampleShaderProgram);

  PostProcessImage* downSampleImageData = arena_alloc_array(&arena, PostProcessImage, 2); 
  Array(PostProcessImage) downSampleImages = create_array(PostProcessImage, downSampleImageData, 2);
  array_index(PostProcessImage, &downSampleImages, 0)->channel = INPUT0;
  array_index(PostProcessImage, &downSampleImages, 0)->uniformName = create_string_from_literal("inputImage");
  array_index(PostProcessImage, &downSampleImages, 1)->channel = OUTPUT0;
  array_index(PostProcessImage, &downSampleImages, 1)->uniformName = create_string_from_literal("outputImage");
  PostProcess downSample = (PostProcess) {downSampleMaterial, downSampleImages};

  // Post process
  ShaderProgram upSampleShaderProgram = create_shader_program();
  attach_shader_to_program(&arena, &upSampleShaderProgram, GL_COMPUTE_SHADER, create_string_from_literal("res/shader/upsample.glsl"));
  finalize_shader_program(&upSampleShaderProgram);
  Material upSampleMaterial = create_material(&arena, &upSampleShaderProgram);

  PostProcessImage* upSampleImageData = arena_alloc_array(&arena, PostProcessImage, 2); 
  Array(PostProcessImage) upSampleImages = create_array(PostProcessImage, upSampleImageData, 2);
  array_index(PostProcessImage, &upSampleImages, 0)->channel = INPUT0;
  array_index(PostProcessImage, &upSampleImages, 0)->uniformName = create_string_from_literal("inputImage");
  array_index(PostProcessImage, &upSampleImages, 1)->channel = OUTPUT0;
  array_index(PostProcessImage, &upSampleImages, 1)->uniformName = create_string_from_literal("outputImage");
  PostProcess upSample = (PostProcess) {upSampleMaterial, upSampleImages};

  // Post process
  ShaderProgram combineShaderProgram = create_shader_program();
  attach_shader_to_program(&arena, &combineShaderProgram, GL_COMPUTE_SHADER, create_string_from_literal("res/shader/combine.glsl"));
  finalize_shader_program(&combineShaderProgram);
  Material combineMaterial = create_material(&arena, &combineShaderProgram);

  PostProcessImage* combineImageData = arena_alloc_array(&arena, PostProcessImage, 3); 
  Array(PostProcessImage) combineImages = create_array(PostProcessImage, combineImageData, 3);
  array_index(PostProcessImage, &combineImages, 0)->channel = INPUT0;
  array_index(PostProcessImage, &combineImages, 0)->uniformName = create_string_from_literal("inputImage");
  array_index(PostProcessImage, &combineImages, 1)->channel = INPUT1;
  array_index(PostProcessImage, &combineImages, 1)->uniformName = create_string_from_literal("inputImage2");
  array_index(PostProcessImage, &combineImages, 2)->channel = OUTPUT0;
  array_index(PostProcessImage, &combineImages, 2)->uniformName = create_string_from_literal("outputImage");
  PostProcess combine = (PostProcess) {combineMaterial, combineImages};

  // Post process
  ShaderProgram splitShaderProgram = create_shader_program();
  attach_shader_to_program(&arena, &splitShaderProgram, GL_COMPUTE_SHADER, create_string_from_literal("res/shader/split.glsl"));
  finalize_shader_program(&splitShaderProgram);
  Material splitMaterial = create_material(&arena, &splitShaderProgram);

  PostProcessImage* splitImageData = arena_alloc_array(&arena, PostProcessImage, 3); 
  Array(PostProcessImage) splitImages = create_array(PostProcessImage, splitImageData, 3);
  array_index(PostProcessImage, &splitImages, 0)->channel = INPUT0;
  array_index(PostProcessImage, &splitImages, 0)->uniformName = create_string_from_literal("inputImage");
  array_index(PostProcessImage, &splitImages, 1)->channel = OUTPUT0;
  array_index(PostProcessImage, &splitImages, 1)->uniformName = create_string_from_literal("outputImage");
  array_index(PostProcessImage, &splitImages, 2)->channel = OUTPUT1;
  array_index(PostProcessImage, &splitImages, 2)->uniformName = create_string_from_literal("outputImage2");
  PostProcess split = (PostProcess) {splitMaterial, splitImages};

  ShaderProgram toneMapShaderProgram = create_shader_program();
  attach_shader_to_program(&arena, &toneMapShaderProgram, GL_COMPUTE_SHADER, create_string_from_literal("res/shader/toneMap.glsl"));
  finalize_shader_program(&toneMapShaderProgram);
  Material toneMapMaterial = create_material(&arena, &toneMapShaderProgram);

  PostProcessImage* toneMapImageData = arena_alloc_array(&arena, PostProcessImage, 2); 
  Array(PostProcessImage) toneMapImages = create_array(PostProcessImage, toneMapImageData, 2);
  array_index(PostProcessImage, &toneMapImages, 0)->channel = INPUT0;
  array_index(PostProcessImage, &toneMapImages, 0)->uniformName = create_string_from_literal("inputImage");
  array_index(PostProcessImage, &toneMapImages, 1)->channel = OUTPUT0;
  array_index(PostProcessImage, &toneMapImages, 1)->uniformName = create_string_from_literal("outputImage");
  PostProcess toneMap = (PostProcess) {toneMapMaterial, toneMapImages};

  /*

  dynamic_array_append(PostProcess, &postProcessList, &downSample);
  dynamic_array_append(PostProcess, &postProcessList, &downSample);
  dynamic_array_append(PostProcess, &postProcessList, &downSample);
  dynamic_array_append(PostProcess, &postProcessList, &downSample);
  dynamic_array_append(PostProcess, &postProcessList, &downSample);
  dynamic_array_append(PostProcess, &postProcessList, &downSample);

  dynamic_array_append(PostProcess, &postProcessList, &upSample);
  dynamic_array_append(PostProcess, &postProcessList, &upSample);
  dynamic_array_append(PostProcess, &postProcessList, &upSample);
  dynamic_array_append(PostProcess, &postProcessList, &upSample);
  dynamic_array_append(PostProcess, &postProcessList, &upSample);
  dynamic_array_append(PostProcess, &postProcessList, &upSample);



  */
  dynamic_array_append(PostProcess, &postProcessList, &split);
  dynamic_array_append(PostProcess, &postProcessList, &combine);
  dynamic_array_append(PostProcess, &postProcessList, &toneMap);
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
