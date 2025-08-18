#ifndef STRING_HEADER
#define STRING_HEADER

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define STRING(s) (String){ .data=s, .len=sizeof(s)-1 }

//_ is used to prevent potential naming conflict
#define CSTRING(string, var)\
  char var##_[string.len+1];\
  string_to_Cstring(string, var##_);\
  const char* var = (const char*) var##_;
  

typedef struct {
  const char* data;
  size_t len;
} String;

#include <stdio.h>
//It is the responsibilty of the caller to insure that ouput has enough space allocated
void string_to_Cstring(String string, char* output) {
  memcpy(output, string.data, string.len);
  output[string.len] = '\0';
}

String string_span(const char* beg, const char* end) {
  String s = {0};
  s.data = beg;
  s.len = beg ? end-beg : 0;
  return s;
}

#include "arena.c"

String string_concatenate(Arena* arena, String a, String b) {
  char* newData = arena_alloc_array(arena, char, a.len+b.len);
  memcpy(newData, a.data, a.len);
  memcpy(newData+a.len, b.data, b.len);
  return (String){newData, a.len+b.len};
}

bool string_equals(const String a, const String b) {
  return a.len == b.len && (!a.len || !memcmp(a.data, b.data, a.len));
}

String string_trim_left(String string) {
  for (; string.len && *string.data<=' '; string.data++, string.len--);
  return string;
}

String string_trim_right(String string) {
  for (; string.len && string.data[string.len-1]<=' '; string.len--);
  return string;
}

bool string_contains(const String parent, const String child) {
  if(child.len > parent.len) return false;
  for(size_t i = 0; i < parent.len-child.len+1; i++) {
    if(string_equals(child, string_span(parent.data+i, parent.data+i+child.len))) return true;
  }
  return false;
}

typedef struct {
  String head;
  String tail;
  bool found;
} Cut;

int32_t string_find(String string, const char c) {
  for(size_t i = 0; i < string.len; i++) {
    if(string.data[i] == c) return i;
  }
  return -1;
}

int32_t string_find_reverse(String string, const char c) {
  for(size_t i = string.len-1; i > 0; i--) {
    if(string.data[i] == c) return i;
  }
  return -1;
}
//Cuts a String into 2 down a first instance of a char c
Cut string_cut(const String string, const char c) {
  if(string.len == 0) return (Cut){0};
  const char* begin = string.data;
  const char* end = begin + string.len;
  const char* cutPtr = begin;
  for(; cutPtr<end && *cutPtr!=c; cutPtr++);
  Cut cut = (Cut){ .found=cutPtr<end, .head=string_span(begin, cutPtr), .tail=string_span(cutPtr+(cutPtr<end), end)};
  return cut;
}

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

#endif
