#ifndef POOL_HEADER
#define POOL_HEADER

#include <malloc.h>
#include <assert.h>
#include <stdint.h>

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
    size_t count;\
    type##Node* end;\
  } type##Pool;\
  \
  type##Pool create_##type##Pool(size_t count) {\
    type##Node* list = (type##Node*) malloc(sizeof(type##Node)*count);\
    assert(list);\
    type##Pool pool = (type##Pool) { .list=list, .count = count, .end = list};\
    for(size_t i = 0; i < count-1; i++) {\
      list[i].prev = &(list[i+1]);\
    }\
    list[count-1].prev = NULL;\
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
  }\
  \
  type* type##Pool_alloc(type##Pool* pool) {\
    type##Node* result = pool->end;\
    assert(result);\
    pool->end = result->prev;\
    result->prev = result;\
    return &(result->data);\
  }

//The node is called node__ just so that there is no conflicting variables
#define foreach(pool, type, var, code)\
  for(size_t i = 0; i < pool.count; i++) {\
    type##Node* node__ = &(pool.list[i]);\
    if(node__->prev != node__) continue;\
    type var = node__->data;\
    code\
  }

#endif
