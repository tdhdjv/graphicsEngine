#!/bin/bash

clang src/main.c glad/glad.o dependencies/stb_image/stb_image.o dependencies/cJSON/cJSON.c\
  -o exe\
  -lm -lglfw -Idependencies/cJSON -Idependencies/stb_image/include -Iglad/include -Idependencies/cglm/include -Idependencies/cgltf\
  -pg -Wall -fsanitize=address -g
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
export LSAN_OPTIONS="suppressions=$PWD/build/asan_suppressions.txt:print_suppressions=0"
