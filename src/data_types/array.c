#ifndef ARRAY_IMPL
#define ARRAY_IMPL

#include <assert.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DynamicArray(type) type##DynamicArray
#define dynamic_array_index(type, array, index) type##_dynamic_array_index(array, index)
#define dynamic_array_append(type, array, value) type##_dynamic_array_append(array, value)
#define dynamic_array_insert(type, array, index, value) type##_dynamic_array_insert(array, index, value)
#define create_dynamic_array(type, capacity) (type##DynamicArray){malloc(capacity * sizeof(type)), 0, capacity}

#define DEFINE_DYNAMIC_ARRAY(type)\
  typedef struct {\
    type* data;\
    size_t length;\
    size_t capacity;\
  } type##DynamicArray;\
  \
  type* type##_dynamic_array_index(const type##DynamicArray* dynamicArray, size_t index) {\
      if(index >= dynamicArray->length) {\
        fprintf(stderr, "Index Out of Bounds");\
        fflush(stderr);\
        abort();\
      }\
      return &dynamicArray->data[index];\
  }\
  \
  void type##_dynamic_array_grow(type##DynamicArray* dynamicArray, size_t amount) {\
    size_t newCapacity = dynamicArray->capacity + amount;\
    type* newData =  realloc(dynamicArray->data, newCapacity * sizeof(type));\
    if(!newData) {\
      fprintf(stderr, "Failed to allocate memory");\
      fflush(stderr);\
      abort();\
    }\
    dynamicArray->capacity = newCapacity;\
    dynamicArray->data = newData;\
  }\
  \
  void type##_dynamic_array_append(type##DynamicArray* dynamicArray, const type* value) {\
    if(dynamicArray->length + 1 > dynamicArray->capacity) {\
       type##_dynamic_array_grow(dynamicArray, dynamicArray->capacity);\
    }\
    dynamicArray->data[dynamicArray->length] = *value;\
    dynamicArray->length++;\
  }\
  \
  void type##_dynamic_array_insert(type##DynamicArray* dynamicArray, size_t index, const type* value) {\
    if(index > dynamicArray->length) {\
      fprintf(stderr, "Index Out Of Bounds");\
      fflush(stderr);\
      abort();\
    }\
    if(dynamicArray->length + 1 > dynamicArray->capacity) {\
       type##_dynamic_array_grow(dynamicArray, dynamicArray->capacity);\
    }\
    memmove(&dynamicArray->data[index] + 1, &dynamicArray->data[index], (dynamicArray->length-index)*sizeof(type));\
    dynamicArray->data[index] = *value;\
    dynamicArray->length++;\
  }

#define Array(type) type##Array
#define array_index(type, array, index) type##_array_index(array, index)
#define create_array(type, data, length) (type##Array){data, length}

#define DEFINE_ARRAY(type)\
  typedef struct {\
    type* data;\
    size_t length;\
  } type##Array;\
  \
  type* type##_array_index(const type##Array* array, size_t index) {\
    if(index >= array->length) {\
      fprintf(stderr, "Index Out of Bounds");\
      fflush(stderr);\
      abort();\
    }\
    return array->data + index;\
  }

#endif
