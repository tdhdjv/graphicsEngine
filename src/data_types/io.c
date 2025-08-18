#ifndef IO_HEADER
#define IO_HEADER

#include <stdio.h>
#include "string.c"
#include "arena.c"

String read_file(Arena* arena, const char *filename) {
  FILE *fileptr;
  fileptr = fopen(filename, "r");
  assert(fileptr);

  fseek(fileptr, 0, SEEK_END);
  size_t n = ftell(fileptr);
  fseek(fileptr, 0, SEEK_SET);

  char *str = arena_alloc(arena,sizeof(char)*n);

  assert(fread(str, 1, n, fileptr) == n);
  fclose(fileptr);
  str[n-1] = '\0';
  String string = {0};
  string.data = str;
  string.len = n;
  return string;
}

void print_string(String string) {
  printf("%.*s", (int)string.len, string.data);
}

#endif
