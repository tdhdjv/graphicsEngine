#ifndef GLTF_LOADER_IMPL
#define GLTF_LOADER_IMPL
#define CGLTF_IMPLEMENTATION

#include <cgltf.h>
#include "scene_define.c"
#include "mesh.c"
#include "data_types/arena.c"
#include "pbr.c"

Mesh load_gltf(Arena* arena) {
  Mesh mesh = {0};
  vec3 albedo = {1.0, 1.0, 1.0}, emissiveness = {0.0, 0.0, 0.0};
  Material material = create_pbr_material_values(arena, albedo, 1.0, 1.0, emissiveness);
  RenderData renderData = {0};
  const cgltf_options options = {0};
  cgltf_data* data = NULL;
  cgltf_result result = cgltf_parse_file(&options, "res/gltf/Lantern/Lantern.gltf", &data);
  if (result != cgltf_result_success) return mesh;

  // Load all buffer data once
  result = cgltf_load_buffers(&options, data, "res/gltf/Lantern/");
  if (result != cgltf_result_success) {
      cgltf_free(data);
      return mesh;
  }
  cgltf_primitive* primitive = data->meshes->primitives;
  if(primitive->material->has_pbr_metallic_roughness) {
    cgltf_material* gltfMaterial = primitive->material;
    cgltf_pbr_metallic_roughness gltfMetallicRounghness = gltfMaterial->pbr_metallic_roughness;
    cgltf_texture* gltfAlbedoTexture = gltfMetallicRounghness.base_color_texture.texture;
    cgltf_texture* gltfMetallicRoughnessTexture = gltfMetallicRounghness.metallic_roughness_texture.texture;
    cgltf_texture* gltfNormalTexture = gltfMaterial->normal_texture.texture;

    char albedoPath[512];
    char metallicRoughnessPath[512];
    char normalPath[512];

    snprintf(albedoPath, sizeof(albedoPath), "%s%s", "res/gltf/Lantern/", gltfAlbedoTexture->image->uri);
    snprintf(metallicRoughnessPath, sizeof(metallicRoughnessPath), "%s%s", "res/gltf/Lantern/", gltfMetallicRoughnessTexture->image->uri);
    snprintf(normalPath, sizeof(normalPath), "%s%s", "res/gltf/Lantern/", gltfNormalTexture->image->uri);

    Texture albedoTexture = create_texture(albedoPath);
    Texture metallicRoughnessTexture = create_texture(metallicRoughnessPath);
    Texture normalTexture = create_texture(normalPath);
    material = create_pbr_material_textured(arena, albedoTexture, metallicRoughnessTexture, normalTexture, 0);
  }

  size_t attribute_count = primitive->attributes_count;
  if(attribute_count == 0) return mesh;

  size_t vertexCount = primitive->attributes[0].data->count;
  size_t indexCount = primitive->indices->count;

  ScratchArena scratchArena = create_scratch_arena(arena);

  vec3* positionData = arena_alloc_array(scratchArena.allocator, vec3, vertexCount);
  vec3* normalData = arena_alloc_array(scratchArena.allocator, vec3, vertexCount);
  vec2* texCoordData = arena_alloc_array(scratchArena.allocator, vec2, vertexCount);
  uint32_t* indexData = arena_alloc_array(scratchArena.allocator, int32_t, indexCount);

  Array(vec3) positionArray =  create_array(vec3, positionData, vertexCount);
  Array(vec3) normalArray =  create_array(vec3, normalData, vertexCount);
  Array(vec2) texCoordArray = create_array(vec2, texCoordData, vertexCount);
  Array(uint32_t) indexArray = create_array(uint32_t, indexData, indexCount);

  Geometry geometry = (Geometry){positionArray, normalArray, texCoordArray, indexArray};

  for(size_t i = 0; i < primitive->attributes_count; i++) {
    cgltf_attribute* attribute = &primitive->attributes[i];

    if (strcmp(attribute->name, "POSITION") == 0) {
      for (size_t v = 0; v < vertexCount; v++) {
        float out[3];
        cgltf_accessor_read_float(attribute->data, v, out, 3);
        positionData[v][0] = out[0];
        positionData[v][1] = out[1];
        positionData[v][2] = out[2];
      }
    }
    else if (strcmp(attribute->name, "NORMAL") == 0) {
      for (size_t v = 0; v < vertexCount; v++) {
        float out[3];
        cgltf_accessor_read_float(attribute->data, v, out, 3);
        normalData[v][0] = out[0];
        normalData[v][1] = out[1];
        normalData[v][2] = out[2];
      }
    }
    else if (strcmp(attribute->name, "TEXCOORD_0") == 0) {
      for (size_t v = 0; v < vertexCount; v++) {
        float out[2];
        cgltf_accessor_read_float(attribute->data, v, out, 2);
        texCoordData[v][0] = out[0];
        texCoordData[v][1] = out[1];
      }
    }
  }
  // Extract indices
  for (size_t i = 0; i < indexCount; i++) {
    indexData[i] = cgltf_accessor_read_index(primitive->indices, i);
  }

  renderData = generate_render_data(arena, &geometry);
  cgltf_free(data);
  release_scratch_arena(scratchArena);
  mesh.material = material;
  mesh.renderData = renderData;

  mat4 modelMatrix = GLM_MAT4_IDENTITY_INIT;
  vec3 translation = {0.0,0.0 , 10.0};
  glm_translate(modelMatrix, translation);
  glm_mat4_copy(modelMatrix, mesh.modelMatrix);

  return mesh;
}

#endif
