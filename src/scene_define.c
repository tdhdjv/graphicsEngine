#ifndef SCENE_DEFINE_IMPL
#define SCENE_DEFINE_IMPL

#include <glad/glad.h>
#include <cglm/cglm.h>
#include "data_types/string.c"
#include "data_types/array.c"

//Data used for a scene
typedef struct {
  vec3 position;
  vec3 facing;
} Camera;

typedef struct {
  String filePath;
  GLenum type;
  GLuint id;
} Shader;

typedef GLuint ShaderProgram;

typedef GLuint Texture;

typedef struct {
  vec3 colorFactor;
  vec3 emissiveFactor;
  float metallicFactor;
  float roughnessFactor;

  Texture colorTexture;
  Texture emissiveTexture;
  Texture metallicRoughnessTexture;

  Texture normalTexture;
  Texture occulsionTexture;
} Material;

#define DEFAULT_MATERIAL {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}};
// The naming standard follows that of GLTF File format and therefore are different from common definitions
typedef struct {
  GLuint type;
  uint8_t vec_dimension;
} Attribute;

// The geometry refers the all the data for the geometry
typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  size_t indexCount;
} Geometry;

// The mesh is contains all the data for rendering geometry and material
// The reason for this seperation is because, you might use the same geometry with a different material
typedef struct {
  Geometry geometry;
  Material material;
  mat4 modelMatrix;
} Mesh;

DEFINE_ARRAY(Mesh)

//Scene refers to a whole which can be seen

typedef struct {
  Array(Mesh) meshList;
  Camera camera;
  GLuint skyBox;
} Scene;

#endif
