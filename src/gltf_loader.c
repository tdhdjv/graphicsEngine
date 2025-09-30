#ifndef GLTF_LOADER_IMPL
#define GLTF_LOADER_IMPL
#define CGLTF_IMPLEMENTATION

#include <cgltf.h>
#include "data_types/arena.c"
#include "data_types/array.c"
#include "data_types/scratch.c"
#include "data_types/string.c"
#include "scene_define.c"
#include "mesh.c"

#define LOAD_ATTRIBUTE(accesor, numComp, srcType, dstPtr) {\
  u32 n = 0; \
  srcType* buffer = (srcType*)accesor->buffer_view->buffer->data + (accesor->buffer_view->offset/sizeof(srcType)) + (accesor->offset/sizeof(srcType));\
  for(u32 k = 0; k < accesor->count; k++) {\
    for(u32 l = 0; l < numComp; l++) {\
      dstPtr[k][l] = (srcType)buffer[n + l];\
    }\
    n += (u32)(accesor->stride/sizeof(srcType));\
  }\
}

Mesh load_gltf_mesh(cgltf_data* data) {
  //mesh processing

}

Mesh load_gtlf(Arena* arena, Array(char) directoryPath, Array(char) gltfFile) {
  Scene scene = {0}; 

  DynamicArray(char) filePath = dynamic_array_create(char, directoryPath.length+gltfFile.length);
  dynamic_array_append_array(char, &filePath, &directoryPath);
  dynamic_array_append_array(char, &filePath, &gltfFile);

  char filePathCstr[256];
  dyn_c_str(filePathCstr, filePath);
  free(filePath.data);

  const cgltf_options options = {0};
  cgltf_data* data = NULL;
  cgltf_result result = cgltf_parse_file(&options, (char*)filePathCstr, &data);
  if(result != cgltf_result_success) {
    fprintf(stderr, "There is no file called :%s\n", filePathCstr);
    fflush(stderr);
    cgltf_free(data);
    return (Mesh){0};
 //   return scene;
  }

  result = cgltf_load_buffers(&options, data, filePathCstr);
  if(result != cgltf_result_success) {
    cgltf_free(data);
    return (Mesh){0};
 //   return scene;
  }
  
  /*
  scene.nodePool = pool_create(SceneNode, data->nodes_count);
  scene.meshPool = pool_create(Mesh, data->meshes_count);
  scene.materialPool = pool_create(Material, data->materials_count);
  */

  for(u32 i = 0; i < data->meshes_count; i++) {
    cgltf_mesh gltfMesh = data->meshes[i];
    for(u32 j = 0; j < gltfMesh.primitives_count; j++) {
      {
        if (__scratch_scratch.capacity == 0) {
          __scratch_scratch = (Arena){malloc(1 << 26), 0, 1 << 26};
          __scratch_offsetStack = (u32Stack){malloc(16), 16, 0};
          ;
        }
        Arena *scratch = &__scratch_scratch;
        u32stack_add(&__scratch_offsetStack, __scratch_scratch.offset,
                     "/home/tdhdjv/dev/graphics_engine/src/gltf_loader.c", 117);
        {
          cgltf_primitive gltfPrimitive = gltfMesh.primitives[j];
          if (gltfPrimitive.type != cgltf_primitive_type_triangles)
            continue;
          cgltf_accessor *indices = gltfPrimitive.indices;
          Geometry *geometry = arena_alloc(scratch, sizeof(Geometry));
          *geometry = (Geometry){0};
          geometry->indexCount = indices->count;
          u32 *indexData =
              arena_alloc(scratch, sizeof(u32) * geometry->indexCount);
          geometry->indices = indexData;
          for (u32 k = 0; k < gltfPrimitive.attributes_count; k++) {
            cgltf_attribute attribute = gltfPrimitive.attributes[k];
            cgltf_accessor *attributeAccessor = attribute.data;
            if (attribute.type == cgltf_attribute_type_position) {
              geometry->vertexCount = attributeAccessor->count;
              vec3 *positions =
                  arena_alloc(scratch, sizeof(vec3) * geometry->vertexCount);
              geometry->positions = positions;
              {
                u32 n = 0;
                float *buffer =
                    (float *)attributeAccessor->buffer_view->buffer->data +
                    (attributeAccessor->buffer_view->offset / sizeof(float)) +
                    (attributeAccessor->offset / sizeof(float));
                for (u32 k = 0; k < attributeAccessor->count; k++) {
                  for (u32 l = 0; l < 3; l++) {
                    geometry->positions[k][l] = (float)buffer[n + l];
                  }
                  n += (u32)(attributeAccessor->stride / sizeof(float));
                }
              };
            } else if (attribute.type == cgltf_attribute_type_tangent) {
              geometry->vertexCount = attributeAccessor->count;
              vec3 *tangents =
                  arena_alloc(scratch, sizeof(vec3) * geometry->vertexCount);
              geometry->tangents = tangents;
              {
                u32 n = 0;
                float *buffer =
                    (float *)attributeAccessor->buffer_view->buffer->data +
                    (attributeAccessor->buffer_view->offset / sizeof(float)) +
                    (attributeAccessor->offset / sizeof(float));
                for (u32 k = 0; k < attributeAccessor->count; k++) {
                  for (u32 l = 0; l < 3; l++) {
                    geometry->tangents[k][l] = (float)buffer[n + l];
                  }
                  n += (u32)(attributeAccessor->stride / sizeof(float));
                }
              };
            } else if (attribute.type == cgltf_attribute_type_normal) {
              geometry->vertexCount = attributeAccessor->count;
              vec3 *normals =
                  arena_alloc(scratch, sizeof(vec3) * geometry->vertexCount);
              geometry->normals = normals;
              {
                u32 n = 0;
                float *buffer =
                    (float *)attributeAccessor->buffer_view->buffer->data +
                    (attributeAccessor->buffer_view->offset / sizeof(float)) +
                    (attributeAccessor->offset / sizeof(float));
                for (u32 k = 0; k < attributeAccessor->count; k++) {
                  for (u32 l = 0; l < 3; l++) {
                    geometry->normals[k][l] = (float)buffer[n + l];
                  }
                  n += (u32)(attributeAccessor->stride / sizeof(float));
                }
              };
            } else if (attribute.type == cgltf_attribute_type_texcoord) {
              geometry->vertexCount = attributeAccessor->count;
              vec2 *texCoords =
                  arena_alloc(scratch, sizeof(vec2) * geometry->vertexCount);
              geometry->texCoords = texCoords;
              {
                u32 n = 0;
                float *buffer =
                    (float *)attributeAccessor->buffer_view->buffer->data +
                    (attributeAccessor->buffer_view->offset / sizeof(float)) +
                    (attributeAccessor->offset / sizeof(float));
                for (u32 k = 0; k < attributeAccessor->count; k++) {
                  for (u32 l = 0; l < 2; l++) {
                    geometry->texCoords[k][l] = (float)buffer[n + l];
                  }
                  n += (u32)(attributeAccessor->stride / sizeof(float));
                }
              };
            }
          }
          cgltf_free(data);
          return mesh_generate_from_geometry(geometry);
        }
        __scratch_scratch.offset = u32stack_pop(
            &__scratch_offsetStack,
            "/home/tdhdjv/dev/graphics_engine/src/gltf_loader.c", 117);
      }
    }
  }
  cgltf_free(data);
  return (Mesh){0};
}

#endif
