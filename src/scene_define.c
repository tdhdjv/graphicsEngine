#ifndef SCENE_DEFINE_IMPL
#define SCENE_DEFINE_IMPL

#include <glad/glad.h>
#include <cglm/cglm.h>
#include "data_types/array.c"
#include "data_types/hashtable.c"
#include "shader_type.c"

//Data used for a scene
typedef struct {
  vec3 position;
  vec3 facing;
} Camera;

typedef GLuint Texture;
typedef GLuint Sampler;

DEFINE_ARRAY(vec3)
DEFINE_ARRAY(vec2)
DEFINE_ARRAY(uint32_t)
// The geometry refers the all the data for the geometry
#define VERTEX_STRIDE 
typedef struct {
  Array(vec3) positions;
  Array(vec3) normals;
  Array(vec2) textureCoordinates;
  Array(uint32_t) indices;
} Geometry;

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  size_t indexCount;
} RenderData;

typedef union {
  bool boolValue;
  int intValue;
  unsigned unsignedValue;
  float floatValue;
  double doubleValue;

  vec2 vec2Value;
  vec3 vec3Value;
  vec4 vec4Value;
  mat2 mat2Value;
  mat3 mat3Value;
  mat4 mat4Value;
} UniformValue;

typedef struct {
  Texture texture;
  Sampler sampler;
} SamplerValue;

uint64_t hashUniformString(String string) {
  uint64_t hash = 0;
  for(int i = 0; i < string.len && i < 2; i++) {
    hash += string.data[i];
    hash <<= 8;
    hash += string.data[string.len-1-i];
    hash <<= 8;
  }
  return hash;
}
DEFINE_HASH_TABLE(String, UniformValue, hashUniformString, string_equals)
DEFINE_HASH_TABLE(String, SamplerValue, hashUniformString, string_equals)

typedef struct {
  const ShaderProgram* shaderProgram;
  HashTable(String, UniformValue) uniformProperties;
  HashTable(String, SamplerValue) samplerProperties;
} Material;

// The mesh is contains all the data for rendering geometry and material
// The reason for this seperation is because, you might use the same geometry with a different material
typedef struct {
  RenderData renderData;
  Material material;
  mat4 modelMatrix;
} Mesh;

DEFINE_ARRAY(Mesh)

//Scene refers to a whole which can be seen
typedef struct {
  Array(Mesh) meshList;
  Camera camera;
  Material skyBoxMaterial;
} Scene;

#endif
