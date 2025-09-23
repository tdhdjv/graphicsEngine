#ifndef GLTF_LOADER_IMPL
#define GLTF_LOADER_IMPL
#define CGLTF_IMPLEMENTATION

#include <cgltf.h>
#include "scene_define.c"
#include "mesh.c"
#include "data_types/arena.c"
#include "pbr.c"
#include "data_types/string.c"

Array(Mesh) load_gltf(Arena* arena, String parentPath, String gltfFile) {
  DynamicString gltfFullPath = create_dynamic_string("", parentPath.len + gltfFile.len);
  dynamic_string_append_string(&gltfFullPath, parentPath);
  dynamic_string_append_string(&gltfFullPath, gltfFile);

  char gltfCstr[parentPath.len+gltfFile.len+1];
  char gltfParentCstr[parentPath.len+1];
  dynamic_string_to_c_str(&gltfFullPath, gltfCstr);
  string_to_c_str(parentPath, gltfParentCstr);

  const cgltf_options options = {0};
  cgltf_data* data = NULL;
  cgltf_result result = cgltf_parse_file(&options, gltfCstr, &data);

  result = cgltf_load_buffers(&options, data, gltfParentCstr);
  if (result != cgltf_result_success) {
      cgltf_free(data);
      return create_array(Mesh, NULL, 0);
  }

  unsigned int primitiveCount = 0;

  for(unsigned int i = 0; i < data->nodes_count; i++)  {
    cgltf_node* node = &(data->nodes[i]);
    cgltf_mesh* mesh = node->mesh;
    if(!mesh) continue;

    for(unsigned int j = 0; j < mesh->primitives_count; j++) {
      if(mesh->primitives[j].type == cgltf_primitive_type_triangles) primitiveCount++;
    }
  }

  Mesh* meshData = arena_alloc_array(arena, Mesh, primitiveCount);
  Array(Mesh) meshList = create_array(Mesh, meshData, primitiveCount);

  for(unsigned i = 0; i < data->nodes_count; i++) {
    cgltf_node* node = &(data->nodes[i]);

    cgltf_mesh* gltfMesh = node->mesh;
    if(!gltfMesh) continue;

    for(unsigned j = 0; j < gltfMesh->primitives_count; j++) {
      // Load all buffer data once
      Mesh mesh = {0};
      vec3 albedo = {1.0, 1.0, 1.0}, emissiveness = {0.0, 0.0, 0.0};
      Material material = create_pbr_material_values(arena, albedo, 1.0, 1.0, emissiveness);
      RenderData renderData = {0};

      cgltf_primitive primitive = gltfMesh->primitives[j];
      if(primitive.material->has_pbr_metallic_roughness) {
        cgltf_material* gltfMaterial = primitive.material;
        cgltf_pbr_metallic_roughness gltfMetallicRounghness = gltfMaterial->pbr_metallic_roughness;
        cgltf_texture* gltfAlbedoTexture = gltfMetallicRounghness.base_color_texture.texture;
        cgltf_texture* gltfMetallicRoughnessTexture = gltfMetallicRounghness.metallic_roughness_texture.texture;
        cgltf_texture* gltfNormalTexture = gltfMaterial->normal_texture.texture;

        char albedoPath[512];
        char metallicRoughnessPath[512];
        char normalPath[512];

        snprintf(albedoPath, sizeof(albedoPath), "%s%s", gltfParentCstr, gltfAlbedoTexture->image->uri);
        snprintf(metallicRoughnessPath, sizeof(metallicRoughnessPath), "%s%s", gltfParentCstr, gltfMetallicRoughnessTexture->image->uri);
        snprintf(normalPath, sizeof(normalPath), "%s%s", gltfParentCstr, gltfNormalTexture->image->uri);

        Texture albedoTexture = create_texture(albedoPath);
        Texture metallicRoughnessTexture = create_texture(metallicRoughnessPath);
        Texture normalTexture = create_texture(normalPath);
        Texture emissiveTexture = 0;

        char emissivePath[512];
        if(gltfMaterial->emissive_texture.texture) {
          cgltf_texture* gltfEmissiveTexture = gltfMaterial->emissive_texture.texture;
          snprintf(emissivePath, sizeof(emissivePath), "%s%s", gltfParentCstr, gltfEmissiveTexture->image->uri);
          emissiveTexture = create_texture(emissivePath);
        }
        material = create_pbr_material_textured(arena, albedoTexture, metallicRoughnessTexture, normalTexture, emissiveTexture);
      }

      size_t attribute_count = primitive.attributes_count;
      if(attribute_count == 0) continue;

      size_t vertexCount = primitive.attributes[0].data->count;
      size_t indexCount = primitive.indices->count;

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

      for(size_t i = 0; i < primitive.attributes_count; i++) {
        cgltf_attribute* attribute = &primitive.attributes[i];

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
        indexData[i] = cgltf_accessor_read_index(primitive.indices, i);
      }

      renderData = generate_render_data(arena, &geometry);
      mesh.material = material;
      mesh.renderData = renderData;

      cgltf_float m[16];
      cgltf_node_transform_world(node, m);
      mat4 modelMatrix = {
        {m[0], m[1], m[2], m[3]},
        {m[4], m[5], m[6], m[7]},
        {m[8], m[9], m[10], m[11]},
        {m[12], m[13], m[14], m[15]}
      };
      glm_mat4_copy(modelMatrix, mesh.modelMatrix);
      release_scratch_arena(scratchArena);

      *array_index(Mesh, &meshList, i) = mesh;
    }
  }
  cgltf_free(data);

  return meshList;
}

#endif
