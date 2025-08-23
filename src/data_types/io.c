#ifndef IO_HEADER
#define IO_HEADER

#include <stdio.h>
#include "string.c"
#include "arena.c"

String read_file(Arena* arena, const char *filename) {
  FILE *fileptr;
  fileptr = fopen(filename, "r");
  if(!fileptr) {
    fprintf(stderr, "%s isn't a valid file path", filename);
    fflush(stderr);
    return (String){0};
  }

  fseek(fileptr, 0, SEEK_END);
  size_t n = ftell(fileptr);
  fseek(fileptr, 0, SEEK_SET);

  char *str = arena_alloc(arena,sizeof(char)*n);

  assert(fread(str, 1, n, fileptr) == n);
  fclose(fileptr);
  String string = {0};
  string.data = str;
  string.len = n;
  return string;
}

void print_string(String string) {
  printf("%.*s", (int)string.len, string.data);
}

#endif
