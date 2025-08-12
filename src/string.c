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

typedef struct {
  String head;
  String tail;
  bool found;
} Cut;

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

#endif
