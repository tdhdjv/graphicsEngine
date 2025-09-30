#ifndef QUEUE_IMPL
#define QUEUE_IMPL

#include "define.c"
#include <string.h>

#define Queue(type) type##Queue
#define queue_create(type, capacity) (type##Queue) {malloc(capacity), capacity, 0, 0}
#define queue_pop(type, queue) type##queue_pop(queue, __FILE__, __LINE__)
#define queue_add(type, queue, value) type##queue_add(queue, value, __FILE__, __LINE__)

#define DEFINE_QUEUE(type) \
  typedef struct {\
    type* data;\
    u32 capacity;\
    u32 start;\
    u32 length;\
  } type##Queue;\
\
  void type##queue_grow(type##Queue* queue, u32 newCapacity, const char* file, const u32 line) {\
    type* newData = realloc(queue->data, newCapacity * sizeof(type));\
    if(!newData) {\
      ASSERT("Failed to allocate memory: \n  at file: %s, line: %u\n", file, line)\
      return;\
    }\
    u32 end = queue->start + queue->length;\
    end %= queue->capacity;\
    if(end < queue->start) {\
      memcpy(queue->data+queue->capacity, queue->data, end*sizeof(type));\
    }\
    queue->capacity = newCapacity;\
    queue->data = newData;\
  }\
  \
  type type##queue_pop(type##Queue* queue, const char* file, const u32 line) {\
    if(queue->length == 0) {\
      ASSERT("Failed to pop an empty queue: \n  at file: %s, line %u\n", file, line);\
    }\
    else queue->length--;\
    type value = queue->data[queue->start];\
    queue->start = (queue->start + 1)%queue->capacity;\
    return value;\
  }\
\
  void type##queue_add(type##Queue* queue, type value, const char* file, const u32 line) {\
    if(queue->length == queue->capacity) {\
      type##queue_grow(queue, queue->capacity*2+1, file, line);\
    }\
    u32 end = queue->start + queue->length;\
    end %= queue->capacity;\
    queue->length++;\
    queue->data[end] = value;\
  }\

#endif
