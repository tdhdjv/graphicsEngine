#ifndef SCRATCH_IMPL
#define SCRATCH_IMPL

#include "arena.c"
#include "data_type_define.c"

#define __scratch_SCRATCH_SIZE 1<<26

static Arena __scratch_scratch = {0, 0 , 0};
static Stack(u32) __scratch_offsetStack = {0};

#define with_scratch(scratchVariableName, ...) {\
  if(__scratch_scratch.capacity == 0) {\
    __scratch_scratch = arena_create(__scratch_SCRATCH_SIZE);\
    __scratch_offsetStack = stack_create(u32, 16);\
  }\
  Arena* scratchVariableName = &__scratch_scratch;\
  stack_add(u32, &__scratch_offsetStack, __scratch_scratch.offset);\
  __VA_ARGS__\
  __scratch_scratch.offset = stack_pop(u32, &__scratch_offsetStack);\
}

#endif

