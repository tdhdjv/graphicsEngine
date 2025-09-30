#ifndef STRING_IMPL
#define STRING_IMPL

#include "define.c"
#include "data_type_define.c"

#ifdef STRING_NO_ERROR
  #define STRING_ASSERT(message, ...)
#else
  #define STRING_ASSERT(message, ...)\
    fprintf(stderr, message, ##__VA_ARGS__);\
    fflush(stderr);\
    abort();
#endif

//specific operations of strings only
#define str(cString) (charArray){cString, sizeof(cString)-1}
#define dyn_str(cString) (charArray){cString, sizeof(cString)-1}

void c_str(char* dst, charArray string) {
  memcpy(dst, string.data, string.length);
  dst[string.length] = '\0';
}

void dyn_c_str(char* dst, charDynamicArray string) {
  memcpy(dst, string.data, string.length);
  dst[string.length] = '\0';
}

#define DEFINE_STR_TO_INT(type) \
i##type str_to_i##type (const Array(char) string) {\
  i##type value = 0;\
  i##type previousValue = 0;\
  i##type sign = 1;\
\
  if(string.length == 0) {\
    STRING_ASSERT("The string '' is can not be converted to i%d", type)\
  }\
\
  size_t i = 0;\
  if(string.data[i] == '-') {\
    sign = -1;\
    i++;\
  }\
\
  for(; i < string.length; i++) {\
    switch(string.data[i]) {\
      case '0':\
      case '1':\
      case '2':\
      case '3':\
      case '4':\
      case '5':\
      case '6':\
      case '7':\
      case '8':\
      case '9':\
        value *= 10;\
        value += string.data[i] - '0';\
        if(value < previousValue) {\
          if(sign > 0) {\
            STRING_ASSERT("Integer Overflow: \nThe value of string %.*s is bigger than the max value of i%d", (i32)string.length, string.data, type)\
          }\
          else {\
            STRING_ASSERT("Integer Underflow: \nThe value of string %.*s is smaller than the min value of i%d", (i32)string.length, string.data, type)\
          }\
        }\
        previousValue = value;\
      break;\
      default:\
        STRING_ASSERT("Invalid char detected at: %zu\nThe string %.*s is can not be converted to i%d", i, (i32)string.length, string.data, type)\
      break;\
    } \
  }\
\
  if(i == 1 && sign == -1) {\
    STRING_ASSERT("The string '-' can not be converted to i%d", type)\
  }\
  return value * sign;\
}\
\
i##type dynamic_str_to_i##type (const DynamicArray(char) string) {\
  i##type value = 0;\
  i##type sign = 1;\
\
  if(string.length == 0) {\
    STRING_ASSERT("The string '' is can not be converted to i%d", type)\
  }\
\
  size_t i = 0;\
  if(string.data[i] == '-') {\
    sign = -1;\
    i++;\
  }\
\
  for(; i < string.length; i++) {\
    switch(string.data[i]) {\
      case '0':\
      case '1':\
      case '2':\
      case '3':\
      case '4':\
      case '5':\
      case '6':\
      case '7':\
      case '8':\
      case '9':\
        if(value > value*10) {\
          STRING_ASSERT("Integer Overflow: \nThe value of string %.*s is bigger than the max value of i%d", (i32)string.length, string.data, type)\
        }\
        value *= 10;\
        value += string.data[i] - '0';\
      break;\
      default:\
        STRING_ASSERT("Invalid char detected at: %zu\nThe string %.*s is can not be converted to i%d", i, (i32)string.length, string.data, type)\
      break;\
    } \
  }\
\
  if(i == 1 && sign == -1) {\
    STRING_ASSERT("The string '-' can not be converted to i%d", type)\
  }\
  return value * sign;\
}

#define DEFINE_STR_TO_UNSIGNED_INT(type) \
\
u##type str_to_u##type (const Array(char) string) {\
  u##type value = 0;\
\
  if(string.length == 0) {\
    STRING_ASSERT("The string '' can not be converted to u%d", type)\
  }\
\
  size_t i = 0;\
  if(string.data[i] == '-') {\
    STRING_ASSERT("Invalid Value:\n The value can't be negative \n The string %.*s can not be converted to u%d", (i32)string.length, string.data, type)\
  }\
\
  for(; i < string.length; i++) {\
    switch(string.data[i]) {\
      case '0':\
      case '1':\
      case '2':\
      case '3':\
      case '4':\
      case '5':\
      case '6':\
      case '7':\
      case '8':\
      case '9':\
        if(value > value*10) {\
          STRING_ASSERT("Integer Overflow: \nThe value of string %.*s is bigger than the max value of i%d", (i32)string.length, string.data, type)\
        }\
        value *= 10;\
        value += string.data[i] - '0';\
      break;\
      default:\
        STRING_ASSERT("Invalid char detected at: %zu\nThe string %.*s is can not be converted to u%d", i, (i32)string.length, string.data, type)\
      break;\
    } \
  }\
\
  return value;\
}\
\
u##type dynamic_str_to_u##type (const DynamicArray(char) string) {\
  u##type value = 0;\
\
  if(string.length == 0) {\
    STRING_ASSERT( "The string '' can not be converted to u%d", type)\
  }\
\
  size_t i = 0;\
  if(string.data[i] == '-') {\
    STRING_ASSERT( "Invalid Value:\n The value can't be negative \n The string %.*s can not be converted to u%d", (i32)string.length, string.data, type)\
  }\
\
  for(; i < string.length; i++) {\
    switch(string.data[i]) {\
      case '0':\
      case '1':\
      case '2':\
      case '3':\
      case '4':\
      case '5':\
      case '6':\
      case '7':\
      case '8':\
      case '9':\
        if(value > value*10) {\
          STRING_ASSERT( "Value Overflow: \nThe value of string %.*s is bigger than the max value of i%d", (i32)string.length, string.data, type)\
          fflush(stderr);\
          abort();\
        }\
        value *= 10;\
        value += string.data[i] - '0';\
      break;\
      default:\
        STRING_ASSERT( "Invalid char detected at: %zu\nThe string %.*s is can not be converted to u%d", i, (i32)string.length, string.data, type)\
        fflush(stderr);\
        abort();\
      break;\
    } \
  }\
\
  return value;\
}

float str_to_float(const Array(char) string) {
  float value = 0.0;
  float sign = 1.0;
  float exp = 0.0;

  if(string.length == 0) {
    STRING_ASSERT( "The string '' is can not be converted to float");
    fflush(stderr);
    abort();
  }

  size_t i = 0;
  if(string.data[i] == '-') {
    sign = -1;
    i++;
  }

  for(; i < string.length; i++) {
    switch(string.data[i]) {
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
        value *= 10.0;
        value += (float)(string.data[i] - '0');
        exp *= 0.1;
        break;
      case '.':
        exp = 1.0;
      break;
      case 'f':
      break;
      default:
        STRING_ASSERT( "Invalid char detected at: %zu\nThe string %.*s is can not be converted to float", i, (i32)string.length, string.data);
        fflush(stderr);
        abort();
      break;
    } 
  }

  if(i == 1 && sign == -1) {
    STRING_ASSERT( "The string '-' can not be converted to float");
    fflush(stderr);
    abort();
  }
  exp = exp == 0.0 ? 1.0 : exp;
  return value * sign * exp;
}

float dynamic_str_to_float(const DynamicArray(char) string) {
  float value = 0.0;
  float sign = 0.0;
  float exp = 0.0;

  if(string.length == 0) {
    STRING_ASSERT( "The string '' is can not be converted to float");
    fflush(stderr);
    abort();
  }

  size_t i = 0;
  if(string.data[i] == '-') {
    sign = -1;
    i++;
  }

  for(; i < string.length; i++) {
    switch(string.data[i]) {
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
        value *= 10.0;
        value += (float)(string.data[i] - '0');
        exp *= 0.1;
        break;
      case '.':
        exp = 1.0;
      break;
      case 'f':
      break;
      default:
        STRING_ASSERT( "Invalid char detected at: %zu\nThe string %.*s is can not be converted to float", i, (i32)string.length, string.data);
        fflush(stderr);
        abort();
      break;
    } 
  }

  if(i == 1 && sign == -1) {
    STRING_ASSERT( "The string '-' can not be converted to float");
    fflush(stderr);
    abort();
  }
  exp = exp == 0.0 ? 1.0 : exp;
  return value * sign * exp;
}

DEFINE_STR_TO_INT(8)
DEFINE_STR_TO_INT(16)
DEFINE_STR_TO_INT(32)
DEFINE_STR_TO_INT(64)

DEFINE_STR_TO_UNSIGNED_INT(8)
DEFINE_STR_TO_UNSIGNED_INT(16)
DEFINE_STR_TO_UNSIGNED_INT(32)
DEFINE_STR_TO_UNSIGNED_INT(64)

#endif


