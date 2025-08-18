#ifndef FILE_IO_HEADER
#define FILE_IO_HEADER

#include <cglm/vec3.h>
#include "../data_types/arena.c"
#include "../data_types/string.c"
#include "../data_types/io.c"
#include <assert.h>

void string_to_vec3(const String string, vec3* result) {
  Cut cut = string_cut(string_trim_left(string), ' ');
  (*result)[0] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[1] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[2] = string_to_float(cut.head);
}

void string_to_vec2(const String string, vec2* result) {
  Cut cut = string_cut(string_trim_left(string), ' ');
  (*result)[0] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[1] = string_to_float(cut.head);
}

void string_to_face(const String string, size_t positionCount, size_t normalCount, size_t uvCount, ivec3* positionIndex, ivec3* normalIndex, ivec3* uvIndex) {
  Cut fields = {0};
  fields.tail = string;
  for(int i = 0; i < 3; i++) {
    fields = string_cut(string_trim_left(fields.tail), ' ');
    Cut element = string_cut(string_trim_left(fields.head), '/');
    (*positionIndex)[i] = string_to_int(element.head);
    element = string_cut(string_trim_left(element.tail), '/');
    (*uvIndex)[i] = string_to_int(element.head);
    element = string_cut(string_trim_left(element.tail), '/');
    (*normalIndex)[i] = string_to_int(element.head);

    if(positionIndex[i] < 0) (*positionIndex)[i] = (int32_t)((*positionIndex[i]) + 1 + positionCount);
    if(uvIndex[i] < 0) (*uvIndex)[i] = (int32_t)((*uvIndex[i]) + 1 + uvCount);
    if(normalIndex[i] < 0) (*normalIndex)[i] = (int32_t)((*normalIndex[i]) + 1 + normalCount);
  }
}

typedef struct {
  vec3* positions;
  size_t positionCount;
  vec3* normals;
  size_t normalCount;
  vec2* uvs;
  size_t uvCount;

  size_t faceCount;
  ivec3* positionIndices;
  ivec3* normalIndices;
  ivec3* uvIndices;
} ObjModel;

ObjModel parse_obj(Arena* arena, const String objSource) {
  ObjModel model = {0};
  Cut lines = {0};

  lines.tail = objSource;
  while(lines.tail.len) {
    lines = string_cut(lines.tail, '\n');
    Cut fields = string_cut(string_trim_right(lines.head), ' ');
    String type = fields.head;

    if(string_equals(STRING("v"), type)) model.positionCount++;
    else if(string_equals(STRING("vt"), type)) model.uvCount++;
    else if(string_equals(STRING("vn"), type)) model.normalCount++;
    else if(string_equals(STRING("f"), type)) model.faceCount++;
  }
  model.positions = arena_alloc_array(arena, vec3, model.positionCount);
  model.normals = arena_alloc_array(arena, vec3, model.normalCount);
  model.uvs = arena_alloc_array(arena, vec2, model.uvCount);
  model.positionIndices = arena_alloc_array(arena, ivec3, model.faceCount);
  model.normalIndices = arena_alloc_array(arena, ivec3, model.faceCount);
  model.uvIndices = arena_alloc_array(arena, ivec3, model.faceCount);

  model.positionCount = model.uvCount = model.normalCount = model.faceCount = 0;
  lines.tail = objSource;
  while(lines.tail.len) {
    lines = string_cut(lines.tail, '\n');
    Cut fields = string_cut(string_trim_right(lines.head), ' ');
    String type = fields.head;
    String data = string_trim_left(fields.tail);
    
    if(string_equals(STRING("v"), type)) string_to_vec3(data, &model.positions[model.positionCount++]);
    else if(string_equals(STRING("vt"), type)) string_to_vec2(data, &model.uvs[model.uvCount++]);   
    else if(string_equals(STRING("vn"), type)) string_to_vec3(data, &model.normals[model.normalCount++]);   
    else if(string_equals(STRING("f"), type))  {
      string_to_face(
        data,
        model.positionCount,
        model.normalCount,
        model.uvCount,
        &model.positionIndices[model.faceCount],
        &model.normalIndices[model.faceCount],
        &model.uvIndices[model.faceCount]
      ); 
      model.faceCount++;
    }
  }
  return model;
}
#endif
