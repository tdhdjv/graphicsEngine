#ifndef PBR_IMPL
#define PBR_IMPL

#include <glad/glad.h>
#include <cglm/cglm.h>

#include "data_types/arena.c"
#include "scene_define.c"
#include "shader.c"
#include "material.c"
#include "environment_map.c"

ShaderProgram pbrShaderProgram; 
Texture pbrBrdfLUT;
Texture preFilterMap;
Texture irradianceMap;

void setup_pbr(Arena* arena, Texture skybox) {
  //pbrBrdfLUT
  pbrBrdfLUT = create_texture("res/PBR/pbrBrdf.png");

  //preFilter
  ShaderProgram preFilterShaderProgram = create_shader_program();
  attach_shader_to_program(arena, &preFilterShaderProgram, GL_VERTEX_SHADER, create_string_from_literal("res/shader/skyboxVertex.glsl"));
  attach_shader_to_program(arena, &preFilterShaderProgram, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/prefilter.glsl"));
  finalize_shader_program(&preFilterShaderProgram);
  Material preFilterMaterial = create_material(arena, &preFilterShaderProgram);

  preFilterMap = create_reflection_probe_env_mip_map(128, skybox, &preFilterMaterial, 5);

  //irradiance map
  ShaderProgram irradianceShaderProgram = create_shader_program();
  attach_shader_to_program(arena, &irradianceShaderProgram, GL_VERTEX_SHADER, create_string_from_literal("res/shader/skyboxVertex.glsl"));
  attach_shader_to_program(arena, &irradianceShaderProgram, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/irradiance.glsl"));
  finalize_shader_program(&irradianceShaderProgram);
  Material irradianceMaterial = create_material(arena, &irradianceShaderProgram);

  irradianceMap = create_reflection_probe_env(128, skybox, &irradianceMaterial);

  //pbr shader
  pbrShaderProgram = create_shader_program();
  attach_shader_to_program(arena, &pbrShaderProgram, GL_VERTEX_SHADER, create_string_from_literal("res/shader/vertex.glsl"));
  attach_shader_to_program(arena, &pbrShaderProgram, GL_FRAGMENT_SHADER, create_string_from_literal("res/shader/fragment.glsl"));
  finalize_shader_program(&pbrShaderProgram);
}

Material create_pbr_material_values(Arena* arena, vec3 albedo, float roughness, float metallic, vec3 emissive) {
  Material pbrMaterial = create_material(arena, &pbrShaderProgram);
  material_set_texture(&pbrMaterial, create_string_from_literal("albedoMap"), whiteTexture);
  //material_set_texture(&pbrMaterial, create_string_from_literal("normalMap"), whiteTexture);
  material_set_texture(&pbrMaterial, create_string_from_literal("roughnessMetallicMap"), whiteTexture);
  material_set_texture(&pbrMaterial, create_string_from_literal("emissiveMap"), whiteTexture);

  material_set_float(&pbrMaterial, create_string_from_literal("metallicFactor"), metallic);
  material_set_float(&pbrMaterial, create_string_from_literal("roughnessFactor"), roughness);
  material_set_vec3(&pbrMaterial, create_string_from_literal("albedoFactor"), albedo);
  material_set_vec3(&pbrMaterial, create_string_from_literal("emissiveFactor"), emissive);

  material_set_texture(&pbrMaterial, create_string_from_literal("prefilterMap"), preFilterMap);
  material_set_texture(&pbrMaterial, create_string_from_literal("irradianceMap"), irradianceMap);
  material_set_texture(&pbrMaterial, create_string_from_literal("brdfLUT"), pbrBrdfLUT);

  return pbrMaterial;
}

Material create_pbr_material_textured(Arena* arena, Texture albedoMap, Texture roughnessMetallicMap, Texture normalMap, Texture emissiveMap) {
  vec3 white_vec = {1.0, 1.0, 1.0};
  Material pbrMaterial = create_material(arena, &pbrShaderProgram);
  material_set_texture(&pbrMaterial, create_string_from_literal("albedoMap"), albedoMap);
  //material_set_texture(&pbrMaterial, create_string_from_literal("normalMap"), normalMap);
  material_set_texture(&pbrMaterial, create_string_from_literal("roughnessMetallicMap"), roughnessMetallicMap);
  material_set_texture(&pbrMaterial, create_string_from_literal("emissiveMap"), emissiveMap);
  
  material_set_float(&pbrMaterial, create_string_from_literal("metallicFactor"), 1.0);
  material_set_float(&pbrMaterial, create_string_from_literal("roughnessFactor"), 1.0);
  material_set_vec3(&pbrMaterial, create_string_from_literal("albedoFactor"), white_vec);
  material_set_vec3(&pbrMaterial, create_string_from_literal("emissiveFactor"), white_vec);

  material_set_texture(&pbrMaterial, create_string_from_literal("prefilterMap"), preFilterMap);
  material_set_texture(&pbrMaterial, create_string_from_literal("irradianceMap"), irradianceMap);
  material_set_texture(&pbrMaterial, create_string_from_literal("brdfLUT"), pbrBrdfLUT);

  return pbrMaterial;
}
#endif
