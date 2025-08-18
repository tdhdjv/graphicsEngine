#ifndef POOL_HEADER
#define POOL_HEADER

#include <assert.h>
#include <stdint.h>

#define Pool(type) type##Pool
#define Node(type) type##Node

#define create_pool(type, data, capacity) create_##type##Pool(data, capacity)
#define free_pool(type, pool) free_##type##Pool(pool)
#define pool_remove(type, pool, data) type##Pool_remove(pool, data)
#define pool_alloc(type, pool) type##Pool_alloc(pool)

#define foreach(pool, type, var, code)\
  for(size_t i = 0, count = 0; count < (pool)->count && i < (pool)->capacity;i++) {\
    type##Node* node__ = &((pool)->list[i]);\
    if(node__->prev != node__) continue;\
    count++;\
    type* var = &(node__->data);\
    code\
  }

//if the data is allocated make it so that prev points to itself
#define DEFINE_POOL(type) \
  typedef struct type##Node type##Node;\
  \
  struct type##Node {\
    type##Node* prev;\
    type data;\
  };\
  \
  typedef struct {\
    type##Node* list;\
    size_t capacity;\
    size_t count;\
    type##Node* end;\
  } type##Pool;\
  \
  type##Pool create_##type##Pool(void* data, size_t capacity) {\
    type##Node* list = (type##Node*) data;\
    assert(list);\
    type##Pool pool = (type##Pool) { .list=list, .capacity=capacity, .count=0, .end=list};\
    for(size_t i = 0; i < capacity-1; i++) {\
      list[i].prev = &(list[i+1]);\
    }\
    list[capacity-1].prev = NULL;\
    return pool;\
  }\
  \
  void free_##type##Pool(type##Pool* pool) { free(pool->list); }\
  \
  void type##Pool_remove(type##Pool* pool, type* data) {\
    size_t i = ((uintptr_t)data - (uintptr_t)pool->list)/sizeof(type##Node);\
    type##Node* node = &(pool->list[i]);\
    assert(&(node->data) == data);\
    node->prev = pool->end; \
    pool->end = node;\
    pool->count--;\
  }\
  \
  type* type##Pool_alloc(type##Pool* pool) {\
    type##Node* result = pool->end;\
    if(!result) {\
      printf("Pool filled to capacity");\
      return NULL;\
    }\
    pool->end = result->prev;\
    result->prev = result;\
    pool->count++;\
    return &(result->data);\
  }

#endif
