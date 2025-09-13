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

Texture textureBuffer1[4];
Texture textureBuffer2[4];

typedef enum {
  INPUT0,
  INPUT1,
  INPUT2,
  INPUT3,

  OUTPUT0,
  OUTPUT1,
  OUTPUT2, 
  OUTPUT3,
} PostProcessChannel;

typedef struct {
  String uniformName;
  PostProcessChannel channel;
} PostProcessImage;

DEFINE_ARRAY(PostProcessImage)

typedef struct {
  Material postProcessMaterial;
  Array(PostProcessImage) images;
} PostProcess;

void setup_post_process(uint16_t width, uint16_t height) {
  previousFrameWidth = width;
  previousFrameHeight = height;
  glGenTextures(4, textureBuffer1);
  for(unsigned i = 0; i < 4; i++) {
    glBindTexture(GL_TEXTURE_2D, textureBuffer1[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  }

  glGenTextures(4, textureBuffer2);
  for(unsigned i = 0; i < 4; i++) {
    glBindTexture(GL_TEXTURE_2D, textureBuffer2[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  }
}

void resize_textures(uint16_t newFrameWidth, uint16_t newFrameHeight) {
  previousFrameWidth = newFrameWidth;
  previousFrameHeight = newFrameHeight;

  for(unsigned i = 0; i < 4; i++) {
    glBindTexture(GL_TEXTURE_2D, textureBuffer1[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, previousFrameWidth, previousFrameHeight, 0, GL_RGBA, GL_FLOAT, NULL);
  }

  for(unsigned i = 0; i < 4; i++) {
    glBindTexture(GL_TEXTURE_2D, textureBuffer2[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, previousFrameWidth, previousFrameHeight, 0, GL_RGBA, GL_FLOAT, NULL);
  }
}

DEFINE_DYNAMIC_ARRAY(PostProcess)

Texture post_process(DynamicArray(PostProcess)* postProcessList, Texture frameTexture, uint16_t frameWidth, uint16_t frameHeight) {
  Texture inputTextures[4];
  Texture outputTextures[4];

  for(unsigned i = 0; i < 4; i++) outputTextures[i] = textureBuffer1[i];
  for(unsigned i = 0; i < 4; i++) inputTextures[i] = textureBuffer2[i];
  //putting frameTexture in the output since the swap is going to happen
  outputTextures[0] = frameTexture;

  if(frameWidth != previousFrameWidth || frameHeight != previousFrameHeight){
    resize_textures(frameWidth, frameHeight);
  }

  for(size_t i = 0; i < postProcessList->length; i++) {
    PostProcess* postProcess = dynamic_array_index(PostProcess, postProcessList, i);
    Material* material = &postProcess->postProcessMaterial;
    glUseProgram(material->shaderProgram->id);
    
    //change the buffer
    for(size_t i = 0; i < postProcess->images.length; i++) {
      PostProcessImage* image = array_index(PostProcessImage, &postProcess->images, i);
      switch(image->channel) {
        case INPUT0:
        case INPUT1:
        case INPUT2:
        case INPUT3:
        {
          unsigned index = image->channel - INPUT0;
          Texture temp = inputTextures[index];
          temp = temp == frameTexture ? textureBuffer1[0]: temp;
          inputTextures[index] = outputTextures[index];
          outputTextures[index] = temp;
        break;
        }
        case OUTPUT0:
        case OUTPUT1:
        case OUTPUT2:
        case OUTPUT3:
        break;
      }
    }
    
    for(size_t i = 0; i < postProcess->images.length; i++) {
      PostProcessImage* image = array_index(PostProcessImage, &postProcess->images, i);
      switch(image->channel) {
        case INPUT0:
        case INPUT1:
        case INPUT2:
        case INPUT3:
          material_set_texture(material, image->uniformName, inputTextures[image->channel-INPUT0]);
        break;
        case OUTPUT0:
        case OUTPUT1:
        case OUTPUT2:
        case OUTPUT3:
          material_set_texture(material, image->uniformName, outputTextures[image->channel-OUTPUT0]);
        break;
      }
    }
    material_push_uniform_values(material);

    glDispatchCompute((frameWidth+7)/8, (frameHeight+7)/8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
  return outputTextures[0];
}

#endif
