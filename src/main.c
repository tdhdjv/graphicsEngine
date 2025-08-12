#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cglm/cam.h>
#include <cglm/cglm.h>
#include <cglm/vec3.h>

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "arena.c"
#include "pool.c"
#include "mesh.c"
#include "string.c"
#include "fileIO.c"

typedef struct {
  vec3 position;
  vec3 facing;
} camera;

void input(GLFWwindow *window, camera *cam, float dt) {
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
  assert(success);

  arena_return_to_mark(&mark);

  return program;
}

GLFWwindow *create_window() {
  GLFWwindow *window;

  if (!glfwInit())
    return NULL;

  glfwWindowHint(GL_MAJOR_VERSION, 4);
  glfwWindowHint(GL_MAJOR_VERSION, 6);

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

  return window;
}

DEFINE_POOL(Mesh)
int main(void) {
  GLFWwindow *window = create_window();
  if (window == NULL)
    return -1;

  // OPENGL SHIT
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);

  char* data = malloc(1<<24);
  Arena arena = create_arena(data, 1<<24);

  // pool
  MeshPool pool = create_MeshPool(100);
  //uint32_t subDiv = 32;
  //Mesh* ico = MeshPool_alloc(&pool);
  Mesh* cube = MeshPool_alloc(&pool);
  //*ico = gen_icosphere(&arena, subDiv);
  *cube = gen_obj_model(&arena, "res/Dragon_8K.obj");

  GLuint shaderProgram = create_shaders(&arena);

  glUseProgram(shaderProgram);
  int8_t time_location = glGetUniformLocation(shaderProgram, "time");
  int8_t model_location = glGetUniformLocation(shaderProgram, "modelMatrix");
  int8_t view_location = glGetUniformLocation(shaderProgram, "viewMatrix");
  int8_t projection_location =
      glGetUniformLocation(shaderProgram, "projectionMatrix");

  camera cam = {.position = {0.0f, 0.0f, 0.0f}, .facing = {0.0f, 0.0f, -1.0f}};

  mat4 modelMatrix = GLM_MAT4_IDENTITY_INIT;
  mat4 projectionMatrix = GLM_MAT4_IDENTITY_INIT;
  glm_perspective(glm_rad(90), 1000.0 / 800.0, 0.1, 100.0, projectionMatrix);

  if (model_location != -1)
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat *)modelMatrix);
  if (projection_location != -1)
    glUniformMatrix4fv(projection_location, 1, GL_FALSE,
                       (GLfloat *)projectionMatrix);

  double previousTime = 0;

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    // updates
    double currentTime = glfwGetTime();
    float dt = (float)(currentTime - previousTime);
    previousTime = currentTime;

    input(window, &cam, dt);

    // pre render
    if (time_location != -1)
      glUniform1f(time_location, glfwGetTime());

    mat4 viewMatrix;
    vec3 origin;
    vec3 up = {0.0f, 1.0f, 0.0f};
    glm_vec3_add(cam.facing, cam.position, origin);

    glm_lookat(cam.position, origin, up, viewMatrix);

    if (view_location != -1)
      glUniformMatrix4fv(view_location, 1, GL_FALSE, (GLfloat *)viewMatrix);

    // renders
    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    foreach(pool, Mesh, m, {
      glBindVertexArray(m.vao);
      glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
    })

      // error handling
    GLenum errorStatus;
    if ((errorStatus = glGetError()) != GL_NO_ERROR) {
      printf("%d", errorStatus);
    }
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  foreach(pool, Mesh, m, {
      glDeleteBuffers(1, &m.vbo);
      glDeleteBuffers(1, &m.ebo);
      glDeleteVertexArrays(1, &m.vao);
  })
  glDeleteProgram(shaderProgram);

  glfwTerminate();
  free_MeshPool(&pool);
  free_arena(&arena);

  return 0;
}
