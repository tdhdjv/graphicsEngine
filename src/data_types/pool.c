#ifndef POOL_HEADER
#define POOL_HEADER

#include "define.c"

#define Pool(type) type##Pool
#define Node(type) pool_##type##_Node

#define pool_create(type, capacity) type##pool_create(capacity, __FILE__, __LINE__)
#define pool_free(type, pool) type##pool_free(pool)
#define pool_remove(type, pool, data) type##pool_remove(pool, data, __FILE__, __LINE__)
#define pool_set(type, pool, value) type##pool_set(pool, value, __FILE__, __LINE__)
#define pool_get(type, pool, index) type##pool_get(pool, index, __FILE__, __LINE__)

//If the index is bigger or equal to the capacity then it is the end of the free list
//freeList is treated like a stack
#define DEFINE_POOL(type) \
  typedef struct {\
    type* data;\
    u32* freeList;\
    u32 freeListHead;\
    u32 capacity;\
    u32 count;\
  } type##Pool;\
  \
  type##Pool type##pool_create(u32 capacity, const char* file, const u32 line) {\
    type* data = malloc(capacity * sizeof(type));\
    u32* freeIndices = malloc(capacity * sizeof(u32));\
    type##Pool pool = {data, freeIndices, capacity-1, capacity, 0};\
    for(u32 i = 0; i < capacity; i++) {\
      freeIndices[i] = capacity-i-1;\
    }\
    return pool;\
  }\
  \
  void type##pool_grow(type##Pool* pool, u32 newCapacity, const char* file, const u32 line) {\
    type* newData = realloc(pool->data, newCapacity * sizeof(type));\
    u32* newFreeList = realloc(pool->freeList, newCapacity * sizeof(u32));\
  \
    if(!newData) {\
      ASSERT("Failed to allocate memory: \n  at file: %s, line: %u\n", file, line)\
      return;\
    }\
    if(!newData) {\
      ASSERT("Failed to allocate memory: \n  at file: %s, line: %u\n", file, line)\
      return;\
    }\
\
    for(u32 i = 0; i < newCapacity-pool->capacity; i++) {\
        newFreeList[pool->freeListHead++] = newCapacity-i-1;\
    }\
    pool->capacity = newCapacity;\
    pool->data = newData;\
    pool->freeList = newFreeList;\
  }\
\
  void type##pool_free(type##Pool* pool) {\
    free(pool->data); \
    free(pool->freeList);\
  }\
\
  void type##pool_remove(type##Pool* pool, u32 index, const char* file, const u32 line) {\
    if(index >= pool->capacity) {\
      ASSERT("Index Out of Bounds: \n %u is not a valid index\n  at file: %s, line: %u", index, file, line)\
    }\
    pool->freeList[++pool->freeListHead] = index;\
  }\
\
  type type##pool_get(type##Pool* pool, u32 index, const char* file, const u32 line) {\
    if(index >= pool->capacity) {\
      ASSERT("Index out of Bounds:\n %u is not a valid index\n  at file: %s, line: %u", index, file, line)\
      return (type){0};\
    }\
    return pool->data[index];\
  }\
\
  u32 type##pool_set(type##Pool* pool, type value, const char* file, const u32 line) {\
    u32 index = pool->freeList[pool->freeListHead];\
    if(pool->freeListHead == 0) {\
      type##pool_grow(pool, pool->capacity*2+1, file, line);\
    }\
    pool->freeListHead--;\
    pool->data[index] = value;\
    return index;\
  }

#endif
