#ifndef BLOOM_IMPL
#define BLOOM_IMPL

#include "data_types/array.c"
#include "data_types/arena.c"
#include "data_types/string.c"
#include "post_process.c"
#include "scene_define.c"
#include "shader.c"
#include "material.c"


ShaderProgram bloomShader;
PostProcess setup_bloom(Arena* arena, Texture threshHoldTexture, Texture* bloomBackBufferTexture) {
  //setup texture for bloom
  glGenTextures(1, bloomBackBufferTexture);
  
  glBindTexture(GL_TEXTURE_2D, *bloomBackBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 960, 540, 0, GL_RGBA, GL_FLOAT, NULL);
  glGenerateMipmap(GL_TEXTURE_2D);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "'fbo' is not complete!");
    fflush(stderr);
    abort();
  }

  bloomShader = create_shader_program();
  attach_shader_to_program(arena, &bloomShader, GL_COMPUTE_SHADER, create_string_from_literal("res/shader/bloom.glsl"));
  finalize_shader_program(&bloomShader);

  Material bloomMaterialH = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterialH, create_string_from_literal("inputImage"), threshHoldTexture);
  material_set_image(&bloomMaterialH, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 0);
  material_set_int(&bloomMaterialH, create_string_from_literal("vertical"), 0);
  material_set_int(&bloomMaterialH, create_string_from_literal("lod"), -1);

  Material bloomMaterialV = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterialV, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterialV, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 0);
  material_set_int(&bloomMaterialV, create_string_from_literal("vertical"), 1);
  material_set_int(&bloomMaterialV, create_string_from_literal("lod"), 0);

  Material bloomMaterial1H = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial1H, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial1H, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 1);
  material_set_int(&bloomMaterial1H, create_string_from_literal("vertical"), 0);
  material_set_int(&bloomMaterial1H, create_string_from_literal("lod"), 0);

  Material bloomMaterial1V = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial1V, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial1V, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 1);
  material_set_int(&bloomMaterial1V, create_string_from_literal("vertical"), 1);
  material_set_int(&bloomMaterial1V, create_string_from_literal("lod"), 1);

  Material bloomMaterial2H = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial2H, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial2H, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 2);
  material_set_int(&bloomMaterial2H, create_string_from_literal("vertical"), 0);
  material_set_int(&bloomMaterial2H, create_string_from_literal("lod"), 1);

  Material bloomMaterial2V = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial2V, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial2V, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 2);
  material_set_int(&bloomMaterial2V, create_string_from_literal("vertical"), 1);
  material_set_int(&bloomMaterial2V, create_string_from_literal("lod"), 2);

  Material bloomMaterial3H = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial3H, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial3H, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 3);
  material_set_int(&bloomMaterial3H, create_string_from_literal("vertical"), 0);
  material_set_int(&bloomMaterial3H, create_string_from_literal("lod"), 2);

  Material bloomMaterial3V = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial3V, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial3V, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 3);
  material_set_int(&bloomMaterial3V, create_string_from_literal("vertical"), 1);
  material_set_int(&bloomMaterial3V, create_string_from_literal("lod"), 3);

  Material bloomMaterial4H = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial4H, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial4H, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 4);
  material_set_int(&bloomMaterial4H, create_string_from_literal("vertical"), 0);
  material_set_int(&bloomMaterial4H, create_string_from_literal("lod"), 3);

  Material bloomMaterial4V = create_material(arena, &bloomShader);
  material_set_texture(&bloomMaterial4V, create_string_from_literal("inputImage"), *bloomBackBufferTexture);
  material_set_image(&bloomMaterial4V, create_string_from_literal("outputImage"), *bloomBackBufferTexture, 4);
  material_set_int(&bloomMaterial4V, create_string_from_literal("lod"), 0);
  material_set_int(&bloomMaterial4V, create_string_from_literal("vertical"), 4);
  
  DynamicArray(Material) postProcessList = create_dynamic_array(Material, 10);
  dynamic_array_append(Material, &postProcessList, &bloomMaterialH);
  dynamic_array_append(Material, &postProcessList, &bloomMaterialV);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial1H);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial1V);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial2H);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial2V);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial3H);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial3V);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial4H);
  dynamic_array_append(Material, &postProcessList, &bloomMaterial4V);

  DispatchSize dispatchSize = {16, 16, 960, 540};
  DispatchSize dispatchSize1 = {16, 16,480, 270};
  DispatchSize dispatchSize2 = {16, 16, 240, 135};
  DispatchSize dispatchSize3 = {16, 16, 120, 67};
  DispatchSize dispatchSize4 = {16, 16, 60, 33};

  DynamicArray(DispatchSize) dispatchList = create_dynamic_array(DispatchSize, 10);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize1);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize1);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize2);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize2);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize3);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize3);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize4);
  dynamic_array_append(DispatchSize, &dispatchList, &dispatchSize4);

  return (PostProcess){postProcessList, dispatchList};
}

#endif
