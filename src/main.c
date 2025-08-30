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
//#include "data_types/pool.c"
//#include "resource.c"
#include "data_types/array.c"
#include "data_types/string.c"
#include "scene_define.c"
#include "shader.c"
#include "material.c"
#include "environment_map.c"
#include "render.c"
#include "mesh.c"

GLFWwindow* window;

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

void GLAPIENTRY
  MessageCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf(stderr, "OpenGL Error: %s\n", message);
  fflush(stderr);
}

void init_window(void) {

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
  window = glfwCreateWindow(1000, 800, "Hello World", NULL, NULL);

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
  /*
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  */
  glViewport(0, 0, 1000, 800);//mode->width, mode->height);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glEnable(GL_DEBUG_OUTPUT);

  glDebugMessageCallback(MessageCallBack, 0);
}

void clean_up(void) {
  
}

int main(void) {

  init_window();
  float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
  };

  unsigned quadVAO, quadVBO;

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  char* data = malloc(1<<27);
  Arena arena = create_arena(data, 1<<27);

  // pool
  //Camera camera = {{0,0,0}, {0, 0, 1}};
  //Mesh* meshArrayData = arena_alloc_array(&arena, Mesh, 49);
  //Array(Mesh) meshes;// =  create_array(Mesh, meshArrayData, 49);
  //vec3 up = {0, 1, 0}, right = {1, 0, 0}; 
  
  /*
  GLuint white = create_texture("res/white.png");
  for(int i = 0; i < 7; i++) { 
    for(int j = 0; j < 7; j++) {
      Mesh mesh = {0};
      mesh.geometry = generate_icosphere_geometry(&arena, 16);
      mesh.material = (Material){{0.9, 0.15, 0.1}, {0.0, 0.0, 0.0}, ((float)i+1)/7, ((float)j+1)/7, white, white, white, 0, 0};
      mat4 modelMatrix = GLM_MAT4_IDENTITY_INIT;
      vec3 translation = {-j*2.2, i*2.2 , 10.0};
      glm_translate(modelMatrix, translation);
      glm_mat4_copy(modelMatrix, mesh.modelMatrix);
      array_set(Mesh, &meshes,(j+7*i) ,mesh);
    }
  }

  */
  //meshes = extract_meshes_from_gltf(&arena, STRING("res/glTF/Helmet/SciFiHelmet.gltf"));//generate_icosphere_geometry(&arena, 32);
  //const char* skyBoxSources[6] = {"res/skybox/right.jpg", "res/skybox/left.jpg", "res/skybox/top.jpg", "res/skybox/bottom.jpg", "res/skybox/front.jpg", "res/skybox/back.jpg"};
  //Scene scene = (Scene){meshes, camera, create_cubeMap(skyBoxSources)};

  //skyBox
  /*
  //frame buffers
  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1000, 800, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1000, 800);  
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // pbr: setup framebuffer
  // ----------------------
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  unsigned int captureFBO;
  glGenFramebuffers(1, &captureFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

  unsigned int captureRBO;
  glGenRenderbuffers(1, &captureRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

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

  GLuint irradiationShader = create_shaders(&arena, "res/shader/skyBoxVertex.glsl", "res/shader/irradiation.glsl");
  unsigned int irradiationMap;
  glGenTextures(1, &irradiationMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradiationMap);

  for(unsigned i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, NULL);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

  glUseProgram(irradiationShader);
  int32_t skyBox1 = glGetUniformLocation(irradiationShader, "environmentMap");
  int32_t projectionLocation1 = glGetUniformLocation(irradiationShader, "projection");
  int32_t viewLocation1 = glGetUniformLocation(irradiationShader, "view"); 

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skyBox);

  if(skyBox1 != -1) glUniform1i(skyBox1, 0);
  if(projectionLocation1 != -1) glUniformMatrix4fv(projectionLocation1, 1, GL_FALSE, (GLfloat*)projectionMatrix);
  glViewport(0, 0, 32, 32);
  for(unsigned i = 0; i < 6; i++) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiationMap, 0);
    if(viewLocation1 != -1) glUniformMatrix4fv(viewLocation1, 1, GL_FALSE, (GLfloat*)captureViews[i]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(cubeMapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  GLuint brdfShader = create_shaders(&arena, "res/shader/postProcessVertex.glsl", "res/shader/brdf.glsl");
  unsigned int brdfLUTTexture;
  glGenTextures(1, &brdfLUTTexture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

  glViewport(0, 0, 512, 512);
  glUseProgram(brdfShader);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, 1000, 800);

  GLuint quadShader = create_shaders(&arena, "res/shader/postProcessVertex.glsl", "res/shader/postProcessFrag.glsl");

  // Loop until the user closes the window
  */

  //setup_default_meshes();
  
  //skybox setup
  setup_environment_map();
  setup_render();
  setup_material();

  ShaderProgram skyboxShader = create_shader_program();
  attach_shader_to_program(&arena, &skyboxShader, GL_VERTEX_SHADER, create_string_from_literal("res/shader/skyboxVertex.glsl"));
  attach_shader_to_program(&arena, &skyboxShader, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/skyboxFragment.glsl"));
  finalize_shader_program(&skyboxShader);
  glUseProgram(skyboxShader.id);

  Material skyBoxMaterial = create_material(&arena, &skyboxShader);
  Texture environmentMap = create_environment_map("res/skybox/right.jpg", "res/skybox/left.jpg", "res/skybox/top.jpg", "res/skybox/bottom.jpg", "res/skybox/front.jpg", "res/skybox/back.jpg");
  material_set_texture(&skyBoxMaterial, create_string_from_literal("environmentMap"), environmentMap);

  Mesh* meshArrayData = arena_alloc_array(&arena, Mesh, 64);
  Array(Mesh) meshes =  create_array(Mesh, meshArrayData, 64);

  ShaderProgram materialShader = create_shader_program();
  attach_shader_to_program(&arena, &materialShader, GL_VERTEX_SHADER, create_string_from_literal("res/shader/vertex.glsl"));
  attach_shader_to_program(&arena, &materialShader, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/fragment.glsl"));
  finalize_shader_program(&materialShader);
  glUseProgram(materialShader.id);
  vec3 color = {1.0, 0.0, 0.0};

  //mesh setup
  for(int i = 0; i < 8; i++) { 
    for(int j = 0; j < 8; j++) {
      Mesh mesh = {0};
      mesh.renderData = generate_icosphere(&arena, 16);
      mesh.material = create_material(&arena, &materialShader);
      material_set_texture(&mesh.material, create_string_from_literal("albedoMap"), whiteTexture);
      material_set_texture(&mesh.material, create_string_from_literal("normalMap"), whiteTexture);
      material_set_texture(&mesh.material, create_string_from_literal("roughtnessMetallicMap"), whiteTexture);
      material_set_texture(&mesh.material, create_string_from_literal("emissiveMap"), whiteTexture);
      material_set_float(&mesh.material, create_string_from_literal("metallicFactor"), (float)(i)/7);
      material_set_float(&mesh.material, create_string_from_literal("roughnessFactor"), (float)(j+1)/7);
      material_set_vec3(&mesh.material, create_string_from_literal("albedoFactor"), color);
      material_set_texture(&mesh.material, create_string_from_literal("prefilterMap"), environmentMap);
      mat4 modelMatrix = GLM_MAT4_IDENTITY_INIT;
      vec3 translation = {-j*2.2, i*2.2 , 10.0};
      glm_translate(modelMatrix, translation);
      glm_mat4_copy(modelMatrix, mesh.modelMatrix);
      *array_index(Mesh, &meshes, i*8 + j) = mesh;
    }
  }

  Camera camera = {{0,0,0}, {0, 0, 1}};

  Scene scene = (Scene){meshes, camera, skyBoxMaterial};

  double previousTime = 0;
  while (!glfwWindowShouldClose(window)) {
    double currentTime = glfwGetTime();
    float dt = (float)(currentTime - previousTime);
    previousTime = currentTime;

    input(window, &(scene.camera), dt);

    render_scene(&scene);
    // updates
    //double currentTime = glfwGetTime();
    //float dt = (float)(currentTime - previousTime);
    //previousTime = currentTime;

    /*
    input(window, &(scene.camera), dt);

    render the scene
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0, 0.21, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_scene(&scene, shaderProgram, skyBoxShaderProgram, prefilterMap, brdfLUTTexture, prefilterMap);

    //post processes
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(quadShader);
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    */
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  glfwTerminate();
  free_arena(&arena);

  return 0;
}
