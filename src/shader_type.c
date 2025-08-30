#ifndef SHADER_TYPE_IMPL
#define SHADER_TYPE_IMPL

#include <glad/glad.h>
#include "data_types/string.c"
#include "data_types/array.c"

typedef enum {
  UNIFORM_TYPE_BOOL = 0,
  UNIFORM_TYPE_INT = 1,
  UNIFORM_TYPE_UNSIGNED_INT = 1,
  UNIFORM_TYPE_FLOAT = 2,
  UNIFORM_TYPE_DOUBLE = 3,
  UNIFORM_TYPE_SAMPLER1D = 4,
  UNIFORM_TYPE_SAMPLER2D = 5,
  UNIFORM_TYPE_SAMPLER3D = 6,
  UNIFORM_TYPE_SAMPLERCUBE = 7,
  UNIFORM_TYPE_VEC2 = 8,
  UNIFORM_TYPE_VEC3 = 9,
  UNIFORM_TYPE_VEC4 = 10,
  UNIFORM_TYPE_MAT2 = 11,
  UNIFORM_TYPE_MAT3 = 12,
  UNIFORM_TYPE_MAT4 = 13,
  UNIFORM_TYPE_INVALID = 14,
} UniformType;

typedef struct {
  UniformType type;
  String name;
  int location;
} Uniform;

DEFINE_DYNAMIC_ARRAY(Uniform)

typedef struct {
  GLuint id;
  DynamicArray(Uniform) uniforms;
} ShaderProgram;

#endif
