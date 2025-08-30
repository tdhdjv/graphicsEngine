#ifndef ARENA_HEADER
#define ARENA_HEADER

#include <stddef.h>
#include <stdint.h>
#include <malloc.h>

#ifndef ARENA_ASSERT
#include <assert.h>
#define ARENA_ASSERT(condition) assert(condition)
#endif //ARENA_ASSERT

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
#endif //DEFAULT_ALIGNMENT

#define ARENA_IS_POWER_OF_TWO(n) (((n) != 0) && (((n) & (n - 1)) == 0))

typedef struct Arena Arena;

struct Arena {
  char *data;
  size_t offset;
  size_t capcity;
};

typedef struct {
  Arena* allocator;
  const size_t markOffset;
} ScratchArena;

uintptr_t align_forward(uintptr_t ptr, uint8_t alignment) {
  uintptr_t offset = ptr % alignment;
  uintptr_t offset_comp = (alignment - offset) % alignment;

  return ptr + offset_comp;
}

void* arena_alloc_align(Arena* arena, size_t n, uint8_t alignment) {
  ARENA_ASSERT(ARENA_IS_POWER_OF_TWO(alignment));
  uintptr_t ptr = (uintptr_t)arena->data + arena->offset;
  uintptr_t aligned_ptr = align_forward(ptr, alignment);

  ARENA_ASSERT(aligned_ptr < (uintptr_t)arena->data + arena->capcity);

  arena->offset = aligned_ptr - (uintptr_t)arena->data + n;
  char* result = (char *)aligned_ptr;

  return result;
}

void* arena_alloc(Arena* arena, size_t n) {
  size_t alignment = DEFAULT_ALIGNMENT;
  return arena_alloc_align(arena, n, alignment);
}

void arena_clear(Arena *arena) { arena->offset = 0; }
Arena create_arena(char*data, size_t capcity) { return (Arena){.data=data, .offset=0, .capcity=capcity}; }

ScratchArena create_scratch_arena(Arena* arena) { return (ScratchArena){.allocator=arena, .markOffset=arena->offset}; }
void release_scratch_arena(ScratchArena mark) { mark.allocator->offset = mark.markOffset; }

void free_arena(Arena* arena) { free(arena->data); }

#define arena_alloc_array(arena, type, n) arena_alloc(arena, sizeof(type) * n)
#define arena_alloc_struct(arena, type) arena_alloc(arena, sizeof(type))

#endif
