#ifndef SCENE_DEFINE_HEADER
#define SCENE_DEFINE_HEADER

/*
 * This file is incharge of defining data types used for descripting a scene
*/

#include <glad/glad.h>
#include <cglm/cglm.h>
#include "../data_types/string.c"
#include "../data_types/array.c"

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

//-----------------------------------------------------------------
// Render Data
//-----------------------------------------------------------------

typedef GLuint Texture;

typedef struct {
  Texture albedoTexture;
} Material;

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
} Mesh;

DEFINE_ARRAY(Mesh)

// The model refers the the data needed to render a single object 
// Mesh and model are separated since, 1 draw call can only handle 1 material
typedef struct {
  Array(Mesh) meshList;
  mat4 modelMatrix;
} Model;

DEFINE_ARRAY(Model)

//Scene refers to a whole which can be seen
typedef struct {
  Array(Model) modelList;
  Camera camera;
} Scene;

//Animation
typedef struct {
 
} SkeletalAnimation;

typedef struct Bone Bone;

struct Bone {
  Bone* children;
  mat4 relativeTransform;
};

#endif
