#ifndef ARRAY_HEADER
#define ARRAY_HEADER

#include <assert.h>
#define Array(type) type##Array
#define array_set(type, array, index, value) type##_array_set(array, index, value)
#define array_get(type, array, index) type##_array_get(array, index)
#define create_array(type, data, capacity) (type##Array){data, capacity}

#define DEFINE_ARRAY(type)\
  typedef struct {\
    type* data;\
    size_t capacity;\
  } type##Array;\
  \
  void type##_array_set(type##Array* array, size_t index, type value) {\
    assert(index < array->capacity);\
    array->data[index] = value;\
  }\
  \
  type type##_array_get(const type##Array* array, size_t index) {\
    assert(index < array->capacity);\
    return array->data[index];\
  }

#endif
