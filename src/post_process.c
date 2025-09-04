#ifndef POST_PROCESS_IMPL
#define POST_PROCESS_IMPL

#include "data_types/array.c"
#include "data_types/string.c"
#include "material.c"
#include "shader_type.c"
#include "scene_define.c"

DEFINE_DYNAMIC_ARRAY(Material)

static uint16_t previousFrameWidth;
static uint16_t previousFrameHeight;

Texture texture1;
Texture texture2;

void setup_post_process(uint16_t width, uint16_t height) {
  previousFrameWidth = width;
  previousFrameHeight = height;
  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
}

void resize_textures(uint16_t newFrameWidth, uint16_t newFrameHeight) {
  previousFrameWidth = newFrameWidth;
  previousFrameHeight = newFrameHeight;

  glBindTexture(GL_TEXTURE_2D, texture1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, previousFrameWidth, previousFrameHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, texture2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, previousFrameWidth, previousFrameHeight, 0, GL_RGBA, GL_FLOAT, NULL);
}

Texture post_process(DynamicArray(Material)* postProcessList, Texture frameTexture, uint16_t frameWidth, uint16_t frameHeight) {
  Texture inputTexture = frameTexture;
  Texture outputTexture = texture1;

  if(frameWidth != previousFrameWidth || frameHeight != previousFrameHeight){
    resize_textures(frameWidth, frameHeight);
  }

  for(size_t i = 0; i < postProcessList->length; i++) {
    Material* material = dynamic_array_index(Material, postProcessList, i);
    glUseProgram(material->shaderProgram->id);
    //material_set_texture(material, create_string_from_literal("inputImage"), frameTexture);
    //material_set_texture(material, create_string_from_literal("outputImage"), outputTexture);
    //material_push_uniform_values(material);

    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glBindImageTexture(1, inputTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);

    Texture temp = inputTexture;
    inputTexture = outputTexture;
    outputTexture = temp != frameTexture ? temp : texture2; 
    glDispatchCompute((frameWidth+7)/8, (frameHeight+7)/8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
  //This is to undo the swap
  outputTexture = inputTexture;
  return outputTexture;
}

#endif
