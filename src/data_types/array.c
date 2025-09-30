#ifndef ARRAY_IMPL
#define ARRAY_IMPL

#include <assert.h>
#include <string.h>

#include "define.c"

#define DynamicArray(type) type##DynamicArray
#define dynamic_array_get(type, array, index) type##dynamic_array_get(array, index, __FILE__, __LINE__)
#define dynamic_array_set(type, array, value, index) type##dynamic_array_set(array, value, index, __FILE__, __LINE__)
#define dynamic_array_append(type, array, value) type##dynamic_array_append(array, value, __FILE__, __LINE__)
#define dynamic_array_insert(type, array, value, index) type##dynamic_array_insert(array, value, index, __FILE__, __LINE__)
#define dynamic_array_append_array(type, array, appendArray) type##dynamic_array_append_array(array, appendArray, __FILE__, __LINE__)
#define dynamic_array_append_dynamic_array(type, array, appendArray) type##dynamic_array_append_dynamic_array(array, appendArray, __FILE__, __LINE__)
#define dynamic_array_insert_array(type, array, index, insertArray) type##dynamic_array_insert_array(array, index, insertArray, __FILE__, __LINE__)
#define dynamic_array_insert_dynamic_array(type, array, index, insertArray) type##dynamic_array_insert_dynamic_array(array, insertArray, index, __FILE__, __LINE__)
#define dynamic_array_pop(type, array, index) type##dynamic_array_pop(array, index, __FILE__, __LINE__)
#define dynamic_array_remove(type, array, value) type##dynamic_array_remove(array, value, __FILE__, __LINE__)
#define dynamic_array_create(type, capacity) (type##DynamicArray){malloc(capacity * sizeof(type)), 0, capacity}
#define dynamic_array_copy(type, destination, source) type##dynamic_array_copy(destination, source, __FILE__, __LINE__)
#define dynamic_array_find(type, array, value) type##dynamic_array_find(array, value)
#define dynamic_array_equals(type, array1, array2) type##dynamic_array_equals(array1, array2)
#define dynamic_array_slice(type, array, start, end) type##dynamic_array_slice(array, start, end, __FILE__, __LINE__)

#define DEFINE_DYNAMIC_ARRAY(type, equalsFunc)\
  typedef struct {\
    type* data;\
    u32 length;\
    u32 capacity;\
  } type##DynamicArray;\
  \
  type type##dynamic_array_get(const type##DynamicArray* dynamicArray, u32 index, const char* file, const u32 line) {\
      if(index >= dynamicArray->length) {\
        ASSERT("Index Out of Bounds Error: \n  index: %u is out of bounds for dynamic array with length of %u \n   at file: %s, line: %d\n", index, dynamicArray->length, file, line)\
        return (type){0};\
      }\
      return dynamicArray->data[index];\
  }\
  \
  void type##dynamic_array_set(const type##DynamicArray* dynamicArray, type value, u32 index, const char* file, const u32 line) {\
      if(index >= dynamicArray->length) {\
        ASSERT("Index Out of Bounds Error: \n  index: %u is out of bounds for dynamic array with length of %u \n   at file: %s, line: %d\n", index, dynamicArray->length, file, line)\
        return;\
      }\
      dynamicArray->data[index] = value;\
  }\
  \
  void type##dynamic_array_grow(type##DynamicArray* dynamicArray, u32 newCapacity, const char* file, const u32 line) {\
    type* newData =  realloc(dynamicArray->data, newCapacity * sizeof(type));\
    if(!newData) {\
      ASSERT("Failed to allocate memory: \n  at file: %s, line: %u\n", file, line)\
      return;\
    }\
    dynamicArray->capacity = newCapacity;\
    dynamicArray->data = newData;\
  }\
  \
  void type##dynamic_array_append(type##DynamicArray* dynamicArray, type value, const char* file, const u32 line) {\
    if(dynamicArray->length + 1 > dynamicArray->capacity) {\
       type##dynamic_array_grow(dynamicArray, dynamicArray->capacity*2+1, file, line);\
    }\
    dynamicArray->data[dynamicArray->length] = value;\
    dynamicArray->length++;\
  }\
  \
  void type##dynamic_array_insert(type##DynamicArray* dynamicArray, const type value, u32 index, const char* file, const u32 line) {\
    if(index > dynamicArray->length) {\
      ASSERT("Index Out of Bounds Error: \n  index: %u is out of bounds for dynamic array with length of %u \n   at file: %s, line: %d\n", index, dynamicArray->length, file, line);\
      return;\
    }\
    if(dynamicArray->length + 1 > dynamicArray->capacity) {\
       type##dynamic_array_grow(dynamicArray, dynamicArray->capacity*2+1, file, line);\
    }\
    memmove(&dynamicArray->data[index] + 1, &dynamicArray->data[index], (dynamicArray->length-index)*sizeof(type));\
    dynamicArray->data[index] = value;\
    dynamicArray->length++;\
  }\
\
  void type##dynamic_array_append_array(type##DynamicArray* dynamicArray, const type##Array* appendArray, const char* file, const u32 line) {\
    if(dynamicArray->length + appendArray->length > dynamicArray->capacity) {\
      u32 size1 = dynamicArray->capacity*2 + 1;\
      u32 size2 = appendArray->length + dynamicArray->length;\
      u32 growSize = size1 > size2 ? size1 : size2;\
      type##dynamic_array_grow(dynamicArray, growSize, file, line);\
    }\
    memcpy(dynamicArray->data + dynamicArray->length, appendArray->data, appendArray->length*sizeof(type)); \
    dynamicArray->length += appendArray->length;\
  }\
\
  void type##dynamic_array_append_dynamic_array(type##DynamicArray* dynamicArray, const type##DynamicArray* appendArray, const char* file, const u32 line) {\
    if(dynamicArray->length + appendArray->length > dynamicArray->capacity) {\
      u32 size1 = dynamicArray->capacity*2 + 1;\
      u32 size2 = appendArray->length + dynamicArray->length;\
      u32 growSize = size1 > size2 ? size1 : size2;\
       type##dynamic_array_grow(dynamicArray, growSize, file, line);\
    }\
    memcpy(dynamicArray->data + dynamicArray->length, appendArray->data, appendArray->length*sizeof(type)); \
    dynamicArray->length += appendArray->length;\
  }\
\
  void type##dynamic_array_insert_array(type##DynamicArray* dynamicArray, type##Array* insertArray, u32 index, const char* file, const u32 line) {\
    if(index > dynamicArray->length) {\
      ASSERT("Index Out of Bounds Error: \n  index: %u is out of bounds for dynamic array with length of %u \n   at file: %s, line: %d\n", index, dynamicArray->length, file, line)\
      return;\
    }\
\
    if(dynamicArray->length + insertArray->length > dynamicArray->capacity) {\
      u32 size1 = dynamicArray->capacity*2 + 1;\
      u32 size2 = insertArray->length + dynamicArray->length;\
      u32 growSize = size1 > size2 ? size1 : size2;\
      type##dynamic_array_grow(dynamicArray, growSize, file, line);\
    }\
    memmove(&dynamicArray->data[index] + insertArray->length, &dynamicArray->data[index], (dynamicArray->length-index)*sizeof(type));\
    memcpy(dynamicArray->data + dynamicArray->length, insertArray->data, insertArray->length*sizeof(type)); \
    dynamicArray->length += insertArray->length;\
  }\
\
  void type##dynamic_array_insert_dynamic_array(type##DynamicArray* dynamicArray, const type##DynamicArray* insertArray, u32 index, const char* file, const u32 line) {\
    if(index > dynamicArray->length) {\
      ASSERT("Index Out of Bounds Error: \n  index: %u is out of bounds for dynamic array with length of %u \n   at file: %s, line: %d\n", index, dynamicArray->length, file, line)\
      return;\
    }\
\
    if(dynamicArray->length + insertArray->length > dynamicArray->capacity) {\
      u32 size1 = dynamicArray->capacity*2 + 1;\
      u32 size2 = insertArray->length + dynamicArray->length;\
      u32 growSize = size1 > size2 ? size1 : size2;\
      type##dynamic_array_grow(dynamicArray, growSize, file, line);\
    }\
    memmove(&dynamicArray->data[index] + insertArray->length, &dynamicArray->data[index], (dynamicArray->length-index)*sizeof(type));\
    memcpy(dynamicArray->data + dynamicArray->length, insertArray->data, insertArray->length*sizeof(type)); \
    dynamicArray->length += insertArray->length;\
  }\
\
  type type##dynamic_array_pop(type##DynamicArray* dynamicArray, u32 index, const char* file, u32 line) {\
    if(index > dynamicArray->length) {\
      ASSERT("Index Out of Bounds Error: \n index: %u is out of bounds for dynamic array with length of %u \n   at file: %s, line: %d\n", index, dynamicArray->length, file, line)\
      return (type){0};\
    }\
    type value = dynamicArray->data[index];\
    memmove(&dynamicArray->data[index], &dynamicArray->data[index+1], (dynamicArray->length-index)*sizeof(type));\
    dynamicArray->length--;\
    return value;\
  }\
\
  type##Array type##dynamic_array_slice(const type##DynamicArray array, u32 start, u32 end, const char* file, const u32 line) {\
    if(start < 0 || start >= array.length) {\
      ASSERT("Index Out of Bounds Error: \n  start index: %u is out of bounds for array with length of %u \n   at file: %s, line: %d\n", start, array.length, file, line)\
      return (type##Array){0};\
    }\
    if(end < start) {\
      ASSERT("Index Out of Bounds Error: \n  end index: %u is smaller than start index: %u \n   at file: %s, line: %d\n", end, start, file, line)\
      return (type##Array){0};\
    } \
    if(end > array.length) {\
      ASSERT("Index Out of Bounds Error: \n  end index: %u is out of bounds for array with length of %u \n   at file: %s, line: %d\n", start, array.length, file, line)\
      return (type##Array){0};\
    }\
    return (type##Array) {array.data + start, end-start};\
  }\
\
  void type##dynamic_array_copy(type##DynamicArray* destination, const type##DynamicArray source, const char* file, const u32 line) {\
    if(destination->data == source.data) {\
      ASSERT("Destination and source point to same data!\n at file: %s, line: %d\n", file, line);\
      return;\
    }\
    if(destination->capacity < source.length) {\
      u32 size1 = destination->capacity*2 +1;\
      u32 size2 = source.length;\
      u32 growSize = size1 > size2 ? size1 : size2;\
      type##dynamic_array_grow(destination, growSize, file, line);\
    }\
    memcpy(destination->data, source.data, source.length*sizeof(type));\
    destination->length = source.length;\
  }\
\
  i64 type##dynamic_array_find(type##DynamicArray* array, type value) {\
    for(u32 i = 0; i < array->length; i++) {\
      if(equalsFunc(array->data[i], value)) return i;\
    }\
    return -1;\
  }\
\
  void type##dynamic_array_remove(type##DynamicArray* dynamicArray, type value, const char* file, u32 line) {\
    i64 index = type##dynamic_array_find(dynamicArray, value);\
    if(index == -1) {\
      ASSERT("Value Error:\n   There is no value in parameter 'value' in this array!\n at file: %s, line: %d\n", file, line)\
      return;\
    }\
    \
    memmove(&dynamicArray->data[index], &dynamicArray->data[index+1], (dynamicArray->length-index)*sizeof(type));\
    dynamicArray->length--;\
    \
  }\
\
  bool type##dynamic_array_equals(type##DynamicArray array1, type##DynamicArray array2) {\
    if(array1.length != array2.length) return false;\
    for(u32 i = 0; i < array1.length; i++) {\
      if(!equalsFunc(array1.data[i], array2.data[i])) return false;\
    }\
    return true;\
  }

#define Array(type) type##Array
#define array_index(type, array, index) type##_array_index(array, index, __FILE__, __LINE__)
#define array_create(type, data, length) (type##Array){data, length}
#define array_copy(type, dest, src) type##array_copy(dest, src, __FILE__, __LINE__)
#define array_find(type, array, value) type##array_find(array, value)
#define array_equals(type, array1, array2) type##array_equals(array1, array2)
#define array_slice(type, array, start, end) type##array_slice(array, start, end, __FILE__, __LINE__)

#define DEFINE_ARRAY(type, equalsFunc)\
  typedef struct {\
    type* data;\
    u32 length;\
  } type##Array;\
  \
  type* type##_array_index(const type##Array* array, u32 index, const char* file, const u32 line) {\
    if(index >= array->length) {\
      ASSERT("Index Out of Bounds Error: \n  index: %u is out of bounds for array with length of %u \n   at file: %s, line: %d\n", index, array->length, file, line)\
      return array->data;\
    }\
    return &array->data[index];\
  }\
\
  type##Array type##array_slice(const type##Array array, u32 start, u32 end, const char* file, const u32 line) {\
    if(start < 0 || start >= array.length) {\
      ASSERT("Index Out of Bounds Error: \n  start index: %u is out of bounds for array with length of %u \n   at file: %s, line: %d\n", start, array.length, file, line)\
      return (type##Array){0};\
    }\
    if(end < start) {\
      ASSERT("Index Out of Bounds Error: \n  end index: %u is smaller than start index: %u \n   at file: %s, line: %d\n", end, start, file, line)\
      return (type##Array){0};\
    } \
    if(end > array.length) {\
      ASSERT("Index Out of Bounds Error: \n  end index: %u is out of bounds for array with length of %u \n   at file: %s, line: %d\n", start, array.length, file, line)\
      return (type##Array){0};\
    }\
    return (type##Array) {array.data + start, end-start};\
  }\
\
  void type##array_copy(type##Array* destination, const type##Array source, const char* file, const u32 line) {\
    if(destination->data == source.data) {\
      ("Destination and source point to same data!\n at file: %s, line: %d\n", file, line);\
    }\
    if(destination->length != source.length) {\
      ("Destination and source doesn't have the same length!\n at file: %s, line: %d\n", file, line);\
    }\
    memcpy(destination->data, source.data, source.length*sizeof(type));\
  }\
\
  i64 type##array_find(type##Array* array, type value) {\
    for(u32 i = 0; i < array->length; i++) {\
      if(equalsFunc(array->data[i], value)) return i;\
    }\
    return -1;\
  }\
\
  bool type##array_equals(type##Array array1, type##Array array2) {\
    if(array1.length != array2.length) return false;\
    for(u32 i = 0; i < array1.length; i++) {\
      if(!equalsFunc(array1.data[i], array2.data[i])) return false;\
    }\
    return true;\
  }

#endif
