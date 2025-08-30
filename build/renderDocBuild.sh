#!/bin/bash

clang src/main.c glad/glad.o dependencies/stb_image/stb_image.o dependencies/cJSON/cJSON.c\
  -o renderDocExe\
  -lm -lglfw -Idependencies/cJSON -Idependencies/stb_image/include -Iglad/include -Idependencies/cglm/include\
  -pg -Wall -Werror -g

env GLFW_USE_WAYLAND=0 $PWD/renderDocExe
