#ifndef OPENGL_UTILS_IMPL
#define OPENGL_UTILS_IMPL

#include <glad/glad.h>
#include <stb_image.h>
#include "data_types/arena.c"
#include "data_types/string.c"

#include "data_types/io.c"

GLuint create_texture(const char* filename) {
  //texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  unsigned char* texData = stbi_load(filename, &width, &height, &nrChannels, STBI_rgb_alpha);
  if(texData) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else {
    printf("Failed to load Image");
  }
  stbi_image_free(texData);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}

GLuint create_cubeMap(const char** const filenames) {
 unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
  int width, height, nrChannels;
  unsigned char *data;  
  for(unsigned int i = 0; i < 6; i++)
  {
    data = stbi_load(filenames[i], &width, &height, &nrChannels, 4);
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    
  return textureID;
}

//========================================================
GLuint create_shaders(Arena* arena, const char* vertexShaderFile, const char* fragmentShaderFile) {
  int success;
  char infoLog[512];

  GLuint program;
  program = glCreateProgram();

  ArenaMark mark = create_arena_mark(arena);

  // Vertex Shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  String vertexSource = read_file(arena, vertexShaderFile);
  CSTRING(vertexSource, CvertexSource);
  glShaderSource(vertexShader, 1, &CvertexSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    fprintf(stderr, "Failed to compile Fragment Shader\n");
    fprintf(stderr, "%s\n", infoLog);
    fflush(stderr);
  }
  glAttachShader(program, vertexShader);
  glDeleteShader(vertexShader);

  arena_return_to_mark(&mark);
  
  mark = create_arena_mark(arena);

  // FragmentShader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  String fragmentSource = read_file(arena, fragmentShaderFile);
  CSTRING(fragmentSource, CfragmentSource);
  glShaderSource(fragmentShader, 1, &CfragmentSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    fprintf(stderr, "Failed to compile Fragment Shader\n");
    fprintf(stderr, "%s\n", infoLog);
    fflush(stderr);
  }

  glAttachShader(program, fragmentShader);
  glDeleteShader(fragmentShader);

  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  arena_return_to_mark(&mark);

  return program;
}

#endif
