#ifndef IO_HEADER
#define IO_HEADER

#include <stdio.h>
#include "string.c"
#include "arena.c"

String read_file(Arena* arena, String filename) {
  FILE *fileptr;
  char filename_c[filename.len + 1];
  string_to_c_str(filename, filename_c);
  fileptr = fopen(filename_c, "r");
  if(!fileptr) {
    fprintf(stderr, "%s isn't a valid file path\n", filename_c);
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

#endif
