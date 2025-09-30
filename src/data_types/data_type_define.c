#ifndef DATA_TYPE_DEFINE_IMPL
#define DATA_TYPE_DEFINE_IMPL

#include "array.c"
#include "stack.c"
#include "hash_table.c"

DEFINE_STACK(u32)
DEFINE_ARRAY(u32, equals)
DEFINE_ARRAY(char, equals)
DEFINE_DYNAMIC_ARRAY(char, equals)

u64 hash_string(Array(char) string) {
  u64 hash = 0;
  for(usize i = 0; i < 2 && i < string.length; i++) {
    hash *= 512;
    hash += string.data[i]*256;
    hash += string.data[string.length-1-i];
  }
  return hash;
}

const Array(char) string_tomb = {0};
const Array(char) string_empty = {0, 1};

bool string_equals(Array(char) a, Array(char) b) {return array_equals(char, a, b);}

DEFINE_HASH_TABLE(charArray, usize, hash_string, string_equals, string_tomb, string_empty)
#endif
