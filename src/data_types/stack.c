#ifndef STACK_IMPL 
#define STACK_IMPL

#include "define.c"
#include <string.h>

#define Stack(type) type##Stack
#define stack_create(type, capacity) (type##Stack){malloc(capacity*sizeof(type)), capacity, 0}; 
#define stack_pop(type, stack) type##stack_pop(stack, __FILE__, __LINE__)
#define stack_add(type, stack, value) type##stack_add(stack, value, __FILE__, __LINE__)

#define DEFINE_STACK(type) \
  typedef struct {\
    type* data;\
    u32 capacity;\
    u32 index;\
  } type##Stack;\
  \
  void type##stack_grow(type##Stack* stack, u32 newCapacity, const char* file, const u32 line) {\
    type* newData = realloc(stack->data, newCapacity * sizeof(type));\
    if(!newData) {\
      ASSERT("Failed to allocate memory: \n  at file: %s, line: %u\n", file, line)\
      return;\
    }\
    stack->capacity = newCapacity;\
    stack->data = newData;\
  }\
\
  type type##stack_pop(type##Stack* stack, const char* file, const u32 line) {\
    if(stack->index == 0) {\
      ASSERT("Failed to pop an empty stack: \n  at file: %s, line:%u\n", file, line)\
    }\
    type value = stack->data[stack->index];\
    if(stack->index > 0) {\
      stack->index--;\
    }\
    return value;\
  }\
\
  void type##stack_add(type##Stack* stack, type value, const char* file, const u32 line) {\
    if(stack->index == stack->capacity-1) {\
      type##stack_grow(stack, stack->capacity*2, file, line);\
    }\
    stack->data[++stack->index] = value;\
  }

#endif
