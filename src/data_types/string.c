#ifndef STRING_IMPL
#define STRING_IMPL

#include <malloc.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct DynamicString DynamicString;
typedef struct String String;

struct DynamicString{
  char* data;
  size_t len;
  size_t capacity;
};

struct String{
  const char* data;
  size_t len;
};

DynamicString create_dynamic_string(const char* stringData, size_t capacity) {
  char* data = malloc(capacity);
  size_t stringLen = strlen(stringData);
  
  if(!data) {
    fprintf(stderr, "Failed to allocate memory");
    fflush(stderr);
    abort();
  }
  if(stringLen > capacity) {
    fprintf(stderr, "The length of String is longer than the capacity");
    fflush(stderr);
    abort();
  }

  memcpy(data, stringData, stringLen);
  return (DynamicString){data, stringLen, capacity};
}

void dynamic_string_grow(DynamicString* dynamicString, size_t amount) {
  size_t newCapacity = dynamicString->capacity + amount;
  char* newData =  realloc(dynamicString->data, newCapacity);
  if(!newData) {
    fprintf(stderr, "Failed to allocate memory");
    fflush(stderr);
    abort();
  }
  dynamicString->capacity = newCapacity;
  dynamicString->data = newData;
}

void dynamic_string_append_cstr(DynamicString* dynamicString, const char* string) {
  size_t stringLen = strlen(string);
  while(dynamicString->len + stringLen > dynamicString->capacity) {
    //grow by 1.5 times
    dynamic_string_grow(dynamicString, dynamicString->capacity/2+1);
  }
  memcpy(dynamicString->data + dynamicString->len, string, stringLen);
  dynamicString->len += stringLen;
}

void dynamic_string_append_string(DynamicString* dynamicString, String string) {
  size_t stringLen = string.len;
  while(dynamicString->len + stringLen > dynamicString->capacity) {
    //grow by 1.5 times
    dynamic_string_grow(dynamicString, dynamicString->capacity/2+1);
  }
  memcpy(dynamicString->data + dynamicString->len, string.data, stringLen);
  dynamicString->len += stringLen;
}

/* This function assumes 2 things
 * 1. that the char* has enough memory allocated!!!
 * 2. that the char* doesn't overlap with the dynamicString->data
*/
void dynamic_string_to_c_str(const DynamicString* dynamicString, char* result) {
  memcpy(result, dynamicString->data, dynamicString->len);
  result[dynamicString->len] = '\0';
};

// String slices / String that are just used for data perposes
#define create_string_from_literal(s) (String){ .data=s, .len=sizeof(s)-1 }

/* This function assumes 2 things
 * 1. that the char* has enough memory allocated!!!
 * 2. that the char* doesn't overlap with the dynamicString->data
*/
void string_to_c_str(String string, char* result) {
  memcpy(result, string.data, string.len);
  result[string.len] = '\0';
}

bool string_equals(String a, String b) {
  return a.len == b.len && (!a.len || ((*a.data == *b.data) && !memcmp(a.data, b.data, a.len)));
}

String string_span(const char* beg, const char* end) {
  String s = {0};
  s.data = beg;
  s.len = beg ? (end==beg ? 0:end-beg) : 0;
  return s;
}

//returns the value in which the string was found return -1 when not found
int32_t string_find_substring(const String parent, const String child) {
  if(child.len > parent.len) return -1;
  for(size_t i = 0; i < parent.len-child.len+1; i++) {
    if(string_equals(child, string_span(parent.data+i, parent.data+i+child.len))) return i;
  }
  return -1;
}

int32_t string_find(String string, const char c) {
  for(size_t i = 0; i < string.len; i++) {
    if(string.data[i] == c) return i;
  }
  return -1;
}

int32_t string_find_reverse(String string, const char c) {
  for(size_t i = string.len-1; i >= 0; i--) {
    if(string.data[i] == c) return i;
  }
  return -1;
}
//Trim the part of the string that contains a char in removals
String string_trim_left(String string, String removals) {
  for (; string.len && string_find(removals, *string.data) != -1; string.data++, string.len--);
  return string;
}

//Trim the part of the string that contains a char in removals
String string_trim_right(String string, String removals) {
  for (; string.len && string_find(removals, string.data[string.len-1]) != -1; string.len--);
  return string;
}

int8_t string_to_int8(const String string) {
  int8_t value = 0;
  char sign = 1;
  for(size_t i = 0; i < string.len; i++) {
    if(value < 0) goto error;
    switch(string.data[i]) {
      case '+': break;
      case '-': sign = -1; break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        value = 10*value + string.data[i] - '0'; 
        break;
      default:
        goto error;
        break;
    }
  }
  return sign*value;

  error:
    #ifdef STRING_ERROR 
    fprintf(stderr, "An Invalid value was given to be parsed into an int");
    fflush(stderr);
    abort();
    #else
    return 0;
    #endif
}

int16_t string_to_int16(const String string) {
  int16_t value = 0;
  char sign = 1;
  for(size_t i = 0; i < string.len; i++) {
    switch(string.data[i]) {
      case '+': break;
      case '-': sign = -1; break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        value = 10*value + string.data[i] - '0'; 
        break;
      default:
        goto error;
        break;
    }
  }
  return sign*value;

  error:
    #ifdef STRING_ERROR 
    fprintf(stderr, "An Invalid value was given to be parsed into an int");
    fflush(stderr);
    abort();
    #else
    return 0;
    #endif
}

int32_t string_to_int32(const String string) {
  int32_t value = 0;
  char sign = 1;
  for(size_t i = 0; i < string.len; i++) {
    switch(string.data[i]) {
      case '+': break;
      case '-': sign = -1; break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        value = 10*value + string.data[i] - '0'; 
        break;
      default:
        goto error;
        break;
    }
  }
  return sign*value;

  error:
    #ifdef STRING_ERROR 
    fprintf(stderr, "An Invalid value was given to be parsed into an int");
    fflush(stderr);
    abort();
    #else
    return 0;
    #endif
}

int64_t string_to_int64(const String string) {
  int64_t value = 0;
  char sign = 1;
  for(size_t i = 0; i < string.len; i++) {
    switch(string.data[i]) {
      case '+': break;
      case '-': sign = -1; break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        value = 10*value + string.data[i] - '0'; 
        break;
      default:
        goto error;
        break;
    }
  }
  return sign*value;

  error:
    #ifdef STRING_ERROR 
    fprintf(stderr, "An Invalid value was given to be parsed into an int");
    fflush(stderr);
    abort();
    #else
    return 0;
    #endif
}


static float exp10i(const int32_t exp) {
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
      case 'e': 
        exp = exp ? exp : 1.0f;
        exp *= exp10i(string_to_int8(string_span(string.data+i+1, string.data + string.len)));
        i = string.len;
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        value = 10.0f*value + (float)(string.data[i] - '0');
        exp *= 0.1f;
        break;
      default:
        #ifdef STRING_ERROR 
        fprintf(stderr, "An Invalid value was given to be parsed into an int");
        fflush(stderr);
        abort();
        #else
        return 0;
        #endif
        break;
    }
  }
  return sign * value * (exp ? exp : 1.0f);
}

typedef struct {
  String head;
  String tail;
  bool found;
} Cut;

//Cuts a String into 2 down a first instance of a char c
Cut string_cut(String string, const char c) {
  if(string.len == 0) return (Cut){0};
  const char* begin = string.data;
  const char* end = begin + string.len;
  const char* cutPtr = begin;
  for(; cutPtr<end && *cutPtr!=c; cutPtr++);
  Cut cut = (Cut){ .found=cutPtr<end, .head=string_span(begin, cutPtr), .tail=string_span(cutPtr+(cutPtr<end), end)};
  return cut;
}

#endif
