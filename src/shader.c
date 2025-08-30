#ifndef SHADER_IMPL
#define SHADER_IMPL

#include "shader_type.c"
#include "data_types/arena.c"
#include "data_types/string.c"
#include "data_types/io.c"

#define create_shader_program(void) (ShaderProgram){glCreateProgram(), create_dynamic_array(Uniform, 16)};

UniformType string_to_uniform_type(String type) {
  //This function assumes that the 'type' is valid!!!
  switch(type.data[0]) {
    case 'b':
      return UNIFORM_TYPE_BOOL;
      break;
    case 'i':
      return UNIFORM_TYPE_INT;
      break;
    case 'u':
      return UNIFORM_TYPE_UNSIGNED_INT;
      break;
    case 'f':
      return UNIFORM_TYPE_FLOAT;
      break;
    case 'd':
      return UNIFORM_TYPE_DOUBLE;
      break;
    case 's':
      //sampler cube
      if(type.data[7] == 'C') {
        return UNIFORM_TYPE_SAMPLERCUBE;
      }
      return UNIFORM_TYPE_SAMPLER1D + (type.data[7] - '1');
      break;
    case 'v':
      return UNIFORM_TYPE_VEC2 + (type.data[3] - '2');
      break;
    default:
      return UNIFORM_TYPE_MAT2 + (type.data[3] - '2');
      break;
  }
  return UNIFORM_TYPE_INVALID;
}

void attach_shader_to_program(Arena* arena, ShaderProgram* shaderProgram, GLenum shaderType, String filePath) {
  int success;
  char infoLog[512];

  GLuint shader = glCreateShader(shaderType);
  String shaderSource = read_file(arena, filePath);
  const String strUniform = create_string_from_literal("uniform");
  const String strDoubleSlash = create_string_from_literal("//");
  //File Parsing
  Cut cut;
  cut.tail = shaderSource;
  while(cut.tail.len > 0) {
    cut = string_cut(cut.tail, '\n'); 
    String line = cut.head;

    int32_t comment_index = string_find_substring(line, strDoubleSlash);
    if(comment_index != -1) continue;

    int32_t uniformStr_index = string_find_substring(line, strUniform);
  
    if(uniformStr_index == -1) continue;

    String stringAfterUniform = string_span(line.data + uniformStr_index, line.data + line.len);
    Cut keywordCut = string_cut(stringAfterUniform, ' ');
    keywordCut = string_cut(string_trim_left(keywordCut.tail, create_string_from_literal(" ")), ' ');
    String type = keywordCut.head;
    String name = string_trim_right(keywordCut.tail, create_string_from_literal(" ;"));
    Uniform uniform = (Uniform){string_to_uniform_type(type), name, 0};
    dynamic_array_append(Uniform, &shaderProgram->uniforms, &uniform);
  }
  
  //Compliation and stuff
  char cShaderSource[shaderSource.len+1];
  string_to_c_str(shaderSource, cShaderSource);
  const char* source = cShaderSource;

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    fprintf(stderr, "%.*s: Failed to compile Shader\n", (int)filePath.len, filePath.data);
    fprintf(stderr, "%s\n", infoLog);
    fflush(stderr);
    abort();
  }
  glAttachShader(shaderProgram->id, shader);
  glDeleteShader(shader);
}

void finalize_shader_program(ShaderProgram* shaderProgram) {
  int success;
  char infoLog[512];
  
  glLinkProgram(shaderProgram->id);
  glGetProgramiv(shaderProgram->id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram->id, 512, NULL, infoLog);
    fprintf(stderr, "Failed to link shader program\n");
    fprintf(stderr, "%s\n", infoLog);
    fflush(stderr);
    abort();
  }

  //setup the uniform locations
  for(size_t i = 0; i < shaderProgram->uniforms.length; i++) {
    Uniform* uniform = dynamic_array_index(Uniform, &shaderProgram->uniforms, i);
    char uniformName[uniform->name.len+1];
    string_to_c_str(uniform->name, uniformName);

    int location = glGetUniformLocation(shaderProgram->id, uniformName);

    uniform->location = location;
    if(location == -1) {
      fprintf(stderr, "uniform %s: is not used\n", uniformName);
      fflush(stderr);
      continue;
    }
  }
}



#endif
