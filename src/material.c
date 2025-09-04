#ifndef MATERIAL_IMPL
#define MATERIAL_IMPL

#include "cglm/mat4.h"
#include "data_types/arena.c"
#include "data_types/hashtable.c"
#include "data_types/string.c"
#include "scene_define.c"
#include "shader_type.c"
#include "opengl_utils.c"

Texture whiteTexture;

void setup_material(Arena* arena) {
  whiteTexture = create_texture("res/white.png");
}

Material create_material(Arena* arena, const ShaderProgram* shaderProgram) {
  size_t uniformCapacity = 2*shaderProgram->uniforms.length;
  KeyValue(String, UniformValue)* uniformData = arena_alloc_array(arena, KeyValue(String, UniformValue), uniformCapacity);
  HashTable(String, UniformValue) uniformProperties = create_hash_table(String, UniformValue, uniformData, uniformCapacity);
  
  size_t samplerCapacity = 2*shaderProgram->uniforms.length;
  KeyValue(String, SamplerValue)* samplerData = arena_alloc_array(arena, KeyValue(String, SamplerValue), samplerCapacity);
  HashTable(String, SamplerValue) samplerProperties = create_hash_table(String, SamplerValue, samplerData, samplerCapacity);
  
  Sampler sampler = 0;
  SamplerValue samplerValue;

  for(size_t i = 0; i < shaderProgram->uniforms.length; i++) {
    Uniform* uniform = dynamic_array_index(Uniform, &shaderProgram->uniforms, i);
    String name = uniform->name;
    switch(uniform->type) {
      case UNIFORM_TYPE_BOOL:
      case UNIFORM_TYPE_INT:
      case UNIFORM_TYPE_FLOAT:
      case UNIFORM_TYPE_DOUBLE:
      case UNIFORM_TYPE_VEC2:
      case UNIFORM_TYPE_VEC3:
      case UNIFORM_TYPE_VEC4:
      case UNIFORM_TYPE_MAT2:
      case UNIFORM_TYPE_MAT3:
      case UNIFORM_TYPE_MAT4:
      case UNIFORM_TYPE_INVALID:
      case UNIFORM_TYPE_IVEC2:
      case UNIFORM_TYPE_IVEC3:
      case UNIFORM_TYPE_IVEC4:
      case UNIFORM_TYPE_UVEC2:
      case UNIFORM_TYPE_UVEC3:
      case UNIFORM_TYPE_UVEC4:
        hash_table_set(String, UniformValue, &uniformProperties, name, (UniformValue){0});
        break;
      case UNIFORM_TYPE_SAMPLER1D:
      case UNIFORM_TYPE_SAMPLER2D:
      case UNIFORM_TYPE_SAMPLER3D:
      case UNIFORM_TYPE_SAMPLERCUBE:
      case UNIFORM_TYPE_IMAGE2D:
        //populate the samplerTable
        samplerValue = (SamplerValue){whiteTexture, sampler++};
        hash_table_set(String, SamplerValue, &samplerProperties, name, samplerValue);
        break;
      }
  }
  return (Material){shaderProgram, uniformProperties, samplerProperties};
}

void material_push_uniform_values(const Material* material) {
  for(size_t i = 0; i < material->shaderProgram->uniforms.length; i++) {
    Uniform* uniform = dynamic_array_index(Uniform, &material->shaderProgram->uniforms, i);
    String name = uniform->name;
    int location = uniform->location;

    if(location == -1) continue;

    UniformValue uniformValue = hash_table_get(String, UniformValue, &material->uniformProperties, name, (UniformValue){0});
    SamplerValue samplerValue = hash_table_get(String, SamplerValue, &material->samplerProperties, name, (SamplerValue){0});

    switch(uniform->type) {
      case UNIFORM_TYPE_BOOL:
        glUniform1i(location, uniformValue.boolValue);
        break;
      case UNIFORM_TYPE_INT:
        glUniform1i(location, uniformValue.intValue);
        break;
      case UNIFORM_TYPE_FLOAT:
        glUniform1f(location, uniformValue.floatValue);
        break;
      case UNIFORM_TYPE_DOUBLE:
        glUniform1d(location, uniformValue.doubleValue);
        break;
      case UNIFORM_TYPE_VEC2:
        glUniform2f(location, uniformValue.vec2Value[0],  uniformValue.vec2Value[1]);
        break;
      case UNIFORM_TYPE_VEC3:
        glUniform3f(location, uniformValue.vec3Value[0],  uniformValue.vec3Value[1], uniformValue.vec3Value[2]);
        break;
      case UNIFORM_TYPE_VEC4:
        glUniform4f(location, uniformValue.vec4Value[0],  uniformValue.vec4Value[1], uniformValue.vec4Value[2], uniformValue.vec4Value[3]);
        break;
      case UNIFORM_TYPE_MAT2:
        glUniformMatrix2fv(location, 1, GL_FALSE, (GLfloat*)uniformValue.mat2Value);
        break;
      case UNIFORM_TYPE_MAT3:
        glUniformMatrix3fv(location, 1, GL_FALSE, (GLfloat*)uniformValue.mat3Value);
        break;
      case UNIFORM_TYPE_MAT4:
        glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat*)uniformValue.mat4Value);
        break;
      case UNIFORM_TYPE_SAMPLER1D:
        glActiveTexture(GL_TEXTURE0+samplerValue.sampler);
        glBindTexture(GL_TEXTURE_1D, samplerValue.texture);
        glUniform1i(location, samplerValue.sampler);
        break;
      case UNIFORM_TYPE_SAMPLER2D:
        glActiveTexture(GL_TEXTURE0+samplerValue.sampler);
        glBindTexture(GL_TEXTURE_2D, samplerValue.texture);
        glUniform1i(location, samplerValue.sampler);
        break;
      case UNIFORM_TYPE_SAMPLER3D:
        glActiveTexture(GL_TEXTURE0+samplerValue.sampler);
        glBindTexture(GL_TEXTURE_3D, samplerValue.texture);
        glUniform1i(location, samplerValue.sampler);
        break;
      case UNIFORM_TYPE_SAMPLERCUBE:
        glActiveTexture(GL_TEXTURE0+samplerValue.sampler);
        glBindTexture(GL_TEXTURE_CUBE_MAP, samplerValue.texture);
        glUniform1i(location, samplerValue.sampler);
        break;
      case UNIFORM_TYPE_INVALID:
        break;
      case UNIFORM_TYPE_IVEC2:
      case UNIFORM_TYPE_IVEC3:
      case UNIFORM_TYPE_IVEC4:
      case UNIFORM_TYPE_UVEC2:
      case UNIFORM_TYPE_UVEC3:
      case UNIFORM_TYPE_UVEC4:
        break;
      case UNIFORM_TYPE_IMAGE2D:
        glBindImageTexture(samplerValue.sampler, samplerValue.texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        break;
      }
  }
}

bool material_contains_uniform(Material* material, String uniformName) {
  return hash_table_index(String, UniformValue, &material->uniformProperties, uniformName);
}

void material_set_mat4(Material* material, String uniformName, mat4 mat4Value) {
  if(!hash_table_contains(String, UniformValue, &material->uniformProperties, uniformName)) {
    fprintf(stderr, "Shader %d There is not uniform called %.*s\n", material->shaderProgram->id, (int)uniformName.len, uniformName.data);
    fflush(stderr);
  }
  UniformValue* uniformValue = hash_table_index(String, UniformValue, &material->uniformProperties, uniformName);
  glm_mat4_copy(mat4Value, uniformValue->mat4Value);
}

void material_set_vec3(Material* material, String uniformName, const vec3 vec3Value) {
  if(!hash_table_contains(String, UniformValue, &material->uniformProperties, uniformName)) {
    fprintf(stderr, "Shader %d There is not uniform called %.*s\n", material->shaderProgram->id, (int)uniformName.len, uniformName.data);
    fflush(stderr);
  }
  UniformValue* uniformValue = hash_table_index(String, UniformValue, &material->uniformProperties, uniformName);
  uniformValue->vec3Value[0] = vec3Value[0];
  uniformValue->vec3Value[1] = vec3Value[1];
  uniformValue->vec3Value[2] = vec3Value[2];
}

void material_set_float(Material* material, String uniformName, float floatValue) {
  if(!hash_table_contains(String, UniformValue, &material->uniformProperties, uniformName)) {
    fprintf(stderr, "Shader %d There is not uniform called %.*s\n", material->shaderProgram->id, (int)uniformName.len, uniformName.data);
    fflush(stderr);
  }
  UniformValue* uniformValue = hash_table_index(String, UniformValue, &material->uniformProperties, uniformName);
  uniformValue->floatValue = floatValue;
}

void material_set_int(Material* material, String uniformName, int intValue) {
  if(!hash_table_contains(String, UniformValue, &material->uniformProperties, uniformName)) {
    fprintf(stderr, "Shader %d There is not uniform called %.*s\n", material->shaderProgram->id, (int)uniformName.len, uniformName.data);
    fflush(stderr);
  }
  UniformValue* uniformValue = hash_table_index(String, UniformValue, &material->uniformProperties, uniformName);
  uniformValue->intValue = intValue;
}

void material_set_texture(Material* material, String uniformName, Texture texture) {
  if(!hash_table_contains(String, SamplerValue, &material->samplerProperties, uniformName)) {
    fprintf(stderr, "Shader %d There is not uniform called %.*s\n", material->shaderProgram->id, (int)uniformName.len, uniformName.data);
    fflush(stderr);
  }
  SamplerValue* samplerValue = hash_table_index(String, SamplerValue, &material->samplerProperties, uniformName);
  samplerValue->texture = texture;
}

#endif
