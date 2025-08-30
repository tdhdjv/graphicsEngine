#ifndef POST_PROCESS_IMPL
#define POST_PROCESS_IMPL

#include "scene_define.c"
#include "data_types/hashtable.c"
#include "data_types/string.c"

uint64_t hashString(String string) {
  return 0;
}

DEFINE_HASH_TABLE(String, UniformValue, hashString, string_equals)

typedef struct {
  ShaderProgram shader;
  HashTable(String, UniformValue) uniforms;
} PostProcess;

#endif
