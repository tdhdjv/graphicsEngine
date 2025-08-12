#!/bin/bash
gcc src/main.c glad/glad.c -o exe -O2 -lm -lglfw -Iglad/include -Idependencies/cglm/include -pg -fsanitize=address -Wall -Werror

LSAN_OPTIONS=suppressions=$PWD/asan_suppressions.txt:print_suppressions=0 ./exe
