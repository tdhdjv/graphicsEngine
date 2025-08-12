#ifndef FILE_IO_HEADER
#define FILE_IO_HEADER

#include "arena.c"
#include "string.c"
#include <assert.h>
#include <cglm/vec3.h>

String read_file(Arena* arena, const char *filename) {
  FILE *fileptr;
  fileptr = fopen(filename, "r");
  assert(fileptr);

  fseek(fileptr, 0, SEEK_END);
  size_t n = ftell(fileptr);
  fseek(fileptr, 0, SEEK_SET);

  char *str = arena_alloc(arena,sizeof(char)*(n));

  assert(fread(str, 1, n, fileptr) == n);
  fclose(fileptr);
  str[n-1] = '\0';
  String string = {0};
  string.data = str;
  string.len = n-1;
  return string;
}

typedef struct {
  vec3* positions;
  size_t positionCount;
  vec3* normals;
  size_t normalCount;

  size_t faceCount;
  ivec3* positionIndices;
  ivec3* normalIndices;
} ObjModel;

int32_t string_to_int(const String string) {
  uint32_t value = 0;
  uint32_t sign = 1;
  for(size_t i = 0; i < string.len; i++) {
    switch(string.data[i]) {
      case '+': break;
      case '-': break;
      default: value = 10*value + string.data[i] - '0';
    }
  }
  return sign*value;
}

float exp10i(const int32_t exp)
{
    float y = 1.0f;
    float x = exp<0 ? 0.1f : exp>0 ? 10.0f : 1.0f;
    int32_t n = exp<0 ? exp : -exp;
    for (; n < -1; n /= 2) {
        y *= n%2 ? x : 1.0f;
        x *= x;
    }
    return x * y;
}

float string_to_float(const String string) {
  float value = 0.0f;
  float sign = 1.0f;
  float exp = 0.0f;

  for(size_t i = 0; i < string.len; i++) {
    switch(string.data[i]) {
      case '+': break;
      case '-': sign = -1; break;
      case '.': exp = 1; break;
      case 'E':
      case 'e': exp = exp ? exp : 1.0f;
                exp *= exp10i(string_to_int(string_span(string.data+i+1, string.data + string.len)));
                i = string.len;
                break;
      default: value = 10.0f*value + (float)(string.data[i] - '0');
               exp *= 0.1f;
    }
  }

  return sign * value * (exp ? exp : 1.0f);
}

void string_to_vec3(const String string, vec3* result) {
  Cut cut = string_cut(string_trim_left(string), ' ');
  (*result)[0] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[1] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[2] = string_to_float(cut.head);
}

void string_to_face(const String string, size_t positionCount, size_t normalCount, ivec3* positionIndex, ivec3* normalIndex) {
  Cut fields = {0};
  fields.tail = string;
  for(int i = 0; i < 3; i++) {
    fields = string_cut(string_trim_left(fields.tail), ' ');
    Cut element = string_cut(string_trim_left(fields.head), '/');
    (*positionIndex)[i] = string_to_int(element.head);
    element = string_cut(string_trim_left(element.tail), '/');
    element = string_cut(string_trim_left(element.tail), '/');
    (*normalIndex)[i] = string_to_int(element.head);

    if(positionIndex[i] < 0) (*positionIndex)[i] = (int32_t)((*positionIndex[i]) + 1 + positionCount);
    if(normalIndex[i] < 0) (*normalIndex)[i] = (int32_t)((*normalIndex[i]) + 1 + normalCount);
  }
}

ObjModel parse_obj(Arena* arena, const String objSource) {
  ObjModel model = {0};
  Cut lines = {0};

  lines.tail = objSource;
  while(lines.tail.len) {
    lines = string_cut(lines.tail, '\n');
    Cut fields = string_cut(string_trim_right(lines.head), ' ');
    String type = fields.head;

    if(string_equals(STRING("v"), type)) model.positionCount++;
    else if(string_equals(STRING("vn"), type)) model.normalCount++;
    else if(string_equals(STRING("f"), type)) model.faceCount++;
  }
  model.positions = arena_alloc_array(arena, vec3, model.positionCount);
  model.normals = arena_alloc_array(arena, vec3, model.normalCount);
  model.positionIndices = arena_alloc_array(arena, ivec3, model.faceCount);
  model.normalIndices = arena_alloc_array(arena, ivec3, model.faceCount);

  model.positionCount = model.normalCount = model.faceCount = 0;
  lines.tail = objSource;
  while(lines.tail.len) {
    lines = string_cut(lines.tail, '\n');
    Cut fields = string_cut(string_trim_right(lines.head), ' ');
    String type = fields.head;
    String data = string_trim_left(fields.tail);
    
    if(string_equals(STRING("v"), type)) string_to_vec3(data, &model.positions[model.positionCount++]);
    else if(string_equals(STRING("vn"), type)) string_to_vec3(data, &model.normals[model.normalCount++]);   
    else if(string_equals(STRING("f"), type))  {
      string_to_face(
        data,
        model.positionCount,
        model.normalCount,
        &model.positionIndices[model.faceCount],
        &model.normalIndices[model.faceCount]
      ); 
      model.faceCount++;
    }
  }
  return model;
}
#endif
