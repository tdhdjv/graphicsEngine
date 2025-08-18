#!/bin/bash
clang src/main.c glad/glad.o dependencies/stb_image/stb_image.o dependencies/cJSON/cJSON.c\
  -o exe\
  -lm -lglfw -Idependencies/cJSON -Idependencies/stb_image/include -Iglad/include -Idependencies/cglm/include\
  -pg -Wall -Werror -fsanitize=address -g

if [ $? -eq 0 ]; then
  ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
  LSAN_OPTIONS=suppressions=$PWD/asan_suppressions.txt:print_suppressions=0
  ./exe
fi
