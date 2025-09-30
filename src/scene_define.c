#ifndef SCENE_DEFINE_IMPL
#define SCENE_DEFINE_IMPL

#include <glad/glad.h>
#include <cglm/cglm.h>
#include "data_types/array.c"
#include "data_types/pool.c"
#include "data_types/hash_table.c"
#include "data_types/string.c"

typedef enum {
  UNIFORM_TYPE_BOOL,
  UNIFORM_TYPE_INT,
  UNIFORM_TYPE_UNSIGNED_INT,
  UNIFORM_TYPE_FLOAT,
  UNIFORM_TYPE_DOUBLE,

  UNIFORM_TYPE_SAMPLER1D,
  UNIFORM_TYPE_SAMPLER2D,
  UNIFORM_TYPE_SAMPLER3D,
  UNIFORM_TYPE_SAMPLER_CUBE,
  UNIFORM_TYPE_SAMPLER2D_RECT,
  UNIFORM_TYPE_SAMPLER1D_ARRAY,
  UNIFORM_TYPE_SAMPLER2D_ARRAY,
  UNIFORM_TYPE_SAMPLER_CUBE_ARRAY,
  UNIFORM_TYPE_SAMPLER_BUFFER,
  UNIFORM_TYPE_SAMPLER2D_MS,
  UNIFORM_TYPE_SAMPLER2D_MS_ARRAY,

  UNIFORM_TYPE_ISAMPLER1D,
  UNIFORM_TYPE_ISAMPLER2D,
  UNIFORM_TYPE_ISAMPLER3D,
  UNIFORM_TYPE_ISAMPLER_CUBE,
  UNIFORM_TYPE_ISAMPLER2D_RECT,
  UNIFORM_TYPE_ISAMPLER1D_ARRAY,
  UNIFORM_TYPE_ISAMPLER2D_ARRAY,
  UNIFORM_TYPE_ISAMPLER_CUBE_ARRAY,
  UNIFORM_TYPE_ISAMPLER_BUFFER,
  UNIFORM_TYPE_ISAMPLER2D_MS,
  UNIFORM_TYPE_ISAMPLER2D_MS_ARRAY,

  UNIFORM_TYPE_USAMPLER1D,
  UNIFORM_TYPE_USAMPLER2D,
  UNIFORM_TYPE_USAMPLER3D,
  UNIFORM_TYPE_USAMPLER_CUBE,
  UNIFORM_TYPE_USAMPLER2D_RECT,
  UNIFORM_TYPE_USAMPLER1D_ARRAY,
  UNIFORM_TYPE_USAMPLER2D_ARRAY,
  UNIFORM_TYPE_USAMPLER_CUBE_ARRAY,
  UNIFORM_TYPE_USAMPLER_BUFFER,
  UNIFORM_TYPE_USAMPLER2D_MS,
  UNIFORM_TYPE_USAMPLER2D_MS_ARRAY,

  UNIFORM_TYPE_VEC2,
  UNIFORM_TYPE_VEC3,
  UNIFORM_TYPE_VEC4,

  UNIFORM_TYPE_IVEC2,
  UNIFORM_TYPE_IVEC3,
  UNIFORM_TYPE_IVEC4,

  UNIFORM_TYPE_UVEC2,
  UNIFORM_TYPE_UVEC3,
  UNIFORM_TYPE_UVEC4,

  UNIFORM_TYPE_DVEC2, 
  UNIFORM_TYPE_DVEC3, 
  UNIFORM_TYPE_DVEC4, 

  UNIFORM_TYPE_BVEC2, 
  UNIFORM_TYPE_BVEC3, 
  UNIFORM_TYPE_BVEC4, 

  UNIFORM_TYPE_MAT2,
  UNIFORM_TYPE_MAT3,
  UNIFORM_TYPE_MAT4,

  UNIFORM_TYPE_IMAGE1D,
  UNIFORM_TYPE_IMAGE2D,
  UNIFORM_TYPE_IMAGE3D,
  UNIFORM_TYPE_IMAGE_CUBE,
  UNIFORM_TYPE_IMAGE2D_RECT,
  UNIFORM_TYPE_IMAGE1D_ARRAY,
  UNIFORM_TYPE_IMAGE2D_ARRAY,
  UNIFORM_TYPE_IMAGE_CUBE_ARRAY,
  UNIFORM_TYPE_IMAGE_BUFFER,
  UNIFORM_TYPE_IMAGE2D_MS,
  UNIFORM_TYPE_IMAGE2D_MS_ARRAY,

  UNIFORM_TYPE_IIMAGE1D,
  UNIFORM_TYPE_IIMAGE2D,
  UNIFORM_TYPE_IIMAGE3D,
  UNIFORM_TYPE_IIMAGE_CUBE,
  UNIFORM_TYPE_IIMAGE2D_RECT,
  UNIFORM_TYPE_IIMAGE1D_ARRAY,
  UNIFORM_TYPE_IIMAGE2D_ARRAY,
  UNIFORM_TYPE_IIMAGE_CUBE_ARRAY,
  UNIFORM_TYPE_IIMAGE_BUFFER,
  UNIFORM_TYPE_IIMAGE2D_MS,
  UNIFORM_TYPE_IIMAGE2D_MS_ARRAY,

  UNIFORM_TYPE_UIMAGE1D,
  UNIFORM_TYPE_UIMAGE2D,
  UNIFORM_TYPE_UIMAGE3D,
  UNIFORM_TYPE_UIMAGE_CUBE,
  UNIFORM_TYPE_UIMAGE2D_RECT,
  UNIFORM_TYPE_UIMAGE1D_ARRAY,
  UNIFORM_TYPE_UIMAGE2D_ARRAY,
  UNIFORM_TYPE_UIMAGE_CUBE_ARRAY,
  UNIFORM_TYPE_UIMAGE_BUFFER,
  UNIFORM_TYPE_UIMAGE2D_MS,
  UNIFORM_TYPE_UIMAGE2D_MS_ARRAY,

  UNIFORM_STRUCT,
} UniformType;

typedef struct {
  UniformType type;
  Array(char) name;
} Uniform;

bool uniform_equals(Uniform a, Uniform b) {return false;}
DEFINE_ARRAY(Uniform, uniform_equals)

typedef struct {
  GLuint id;
  Array(Uniform) uniforms;
} ShaderProgram;

//hash table setup

typedef struct {
  ShaderProgram* shaderProgram;
  HashTable(charArray, usize) uniformDataLocation;
  //not using array since it is more of just a stream of data when an actual array
  byte* uniformData;
} Material;

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  vec3 boxMin;
  vec3 boxMax;

  u32 indexCount;
} Mesh;

typedef struct {
  vec3* positions;
  vec3* tangents;
  vec3* normals;
  vec2* texCoords;

  u32* indices;

  u32 vertexCount;
  u32 indexCount;

  vec3 boxMin;
  vec3 boxMax;
} Geometry;

typedef struct SceneNode SceneNode;

struct SceneNode {
  u32 parentIndex;
  union {
    Array(u32) childrenIndices;
    struct {
      u32 meshIndex;
      u32 materialIndex;
    } renderObjects;
  };
  vec3 translation;
  vec3 rotation;
  vec3 scale;
};

DEFINE_POOL(SceneNode)
DEFINE_POOL(Mesh)
DEFINE_POOL(Material)

//Data used for a scene
typedef struct {
  vec3 position;
  vec3 facing;
} Camera;

typedef struct {
  Camera camera;
  Material skyBoxMaterial;
  u32 rootIndex;
  Pool(SceneNode) nodePool;
  Pool(Mesh) meshPool;
  Pool(Material) materialPool;
} Scene;

DEFINE_ARRAY(mat4, equals)

typedef struct {
  Mesh* mesh;
  Material* material;
  u32 instanceCount;
  Array(mat4) modelMatrices;
} DrawCall;

#endif
