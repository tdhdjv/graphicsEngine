#ifndef POST_PROCESS_IMPL
#define POST_PROCESS_IMPL

#include <stdint.h>
#include "data_types/array.c"
#include "scene_define.c"
#include "material.c"

typedef struct {
  uint8_t localSizeX;
  uint8_t localSizeY;
  
  uint16_t imageSizeX;
  uint16_t imageSizeY;
} DispatchSize;

DEFINE_DYNAMIC_ARRAY(Material);
DEFINE_DYNAMIC_ARRAY(DispatchSize);

typedef struct {
  DynamicArray(Material) materials;
  DynamicArray(DispatchSize) dispatchSizes;
} PostProcess;

DEFINE_DYNAMIC_ARRAY(PostProcess)

void post_process(DynamicArray(Material)* postProcessList, DynamicArray(DispatchSize)* dispatchSizeList) {
  for(size_t i = 0; i < postProcessList->length; i++) {
    Material* material = dynamic_array_index(Material, postProcessList, i);
    DispatchSize* dispatchSize = dynamic_array_index(DispatchSize, dispatchSizeList, i);

    glUseProgram(material->shaderProgram->id);
    material_set_int(material, create_string_from_literal("inputWidth"), (int)dispatchSize->imageSizeX);
    material_set_int(material, create_string_from_literal("inputHeight"), (int)dispatchSize->imageSizeY);
    material_push_uniform_values(material);
    int workGroupX = (dispatchSize->imageSizeX+dispatchSize->localSizeX-1)/dispatchSize->localSizeX; 
    int workGroupY = (dispatchSize->imageSizeY+dispatchSize->localSizeY-1)/dispatchSize->localSizeY; 
    glDispatchCompute(workGroupX, workGroupY,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
}

#endif
