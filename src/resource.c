// This file is incharge of every loading and writing of files
#ifndef RESOURCE_IMPL
#define RESOURCE_IMPL

#include <glad/glad.h>
#include <cglm/vec3.h>
#include "data_types/arena.c"
#include "data_types/string.c"
#include <assert.h>

//=====================================
// stuff that will be used frequently (ex: quads, cubes, white texture, etc)

static unsigned cubeMapVAO, quadVAO;
void init_default_values(void) {
  float cubeMapVertices[] = {
        // positions          
-1.0f,  1.0f, -1.0f,
-1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f, -1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f, -1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

-1.0f,  1.0f, -1.0f,
 1.0f,  1.0f, -1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
 1.0f, -1.0f,  1.0f
  };

  float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
  };

  unsigned temp, cubeMapVBO, quadVBO;
  glGenVertexArrays(1, &temp);
  glGenBuffers(1, &cubeMapVBO);
  glBindVertexArray(temp);
  glBindBuffer(GL_ARRAY_BUFFER, cubeMapVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeMapVertices), &cubeMapVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  cubeMapVAO = temp;

  glGenVertexArrays(1, &temp);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(temp);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  quadVAO = temp;
}

//String handling
void string_to_vec3(const String string, vec3* result) {
  Cut cut = string_cut(string_trim_left(string), ' ');
  (*result)[0] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[1] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[2] = string_to_float(cut.head);
}

void string_to_vec2(const String string, vec2* result) {
  Cut cut = string_cut(string_trim_left(string), ' ');
  (*result)[0] = string_to_float(cut.head);
  cut = string_cut(string_trim_left(cut.tail), ' ');
  (*result)[1] = string_to_float(cut.head);
}

void string_to_face(const String string, size_t positionCount, size_t normalCount, size_t uvCount, ivec3* positionIndex, ivec3* normalIndex, ivec3* uvIndex) {
  Cut fields = {0};
  fields.tail = string;
  for(int i = 0; i < 3; i++) {
    fields = string_cut(string_trim_left(fields.tail), ' ');
    Cut element = string_cut(string_trim_left(fields.head), '/');
    (*positionIndex)[i] = string_to_int(element.head);
    element = string_cut(string_trim_left(element.tail), '/');
    (*uvIndex)[i] = string_to_int(element.head);
    element = string_cut(string_trim_left(element.tail), '/');
    (*normalIndex)[i] = string_to_int(element.head);

    if(positionIndex[i] < 0) (*positionIndex)[i] = (int32_t)((*positionIndex[i]) + 1 + positionCount);
    if(uvIndex[i] < 0) (*uvIndex)[i] = (int32_t)((*uvIndex[i]) + 1 + uvCount);
    if(normalIndex[i] < 0) (*normalIndex)[i] = (int32_t)((*normalIndex[i]) + 1 + normalCount);
  }
}

typedef struct {
  vec3* positions;
  size_t positionCount;
  vec3* normals;
  size_t normalCount;
  vec2* uvs;
  size_t uvCount;

  size_t faceCount;
  ivec3* positionIndices;
  ivec3* normalIndices;
  ivec3* uvIndices;
} ObjModel;

ObjModel parse_obj(Arena* arena, const String objSource) {
  ObjModel model = {0};
  Cut lines = {0};

  lines.tail = objSource;
  while(lines.tail.len) {
    lines = string_cut(lines.tail, '\n');
    Cut fields = string_cut(string_trim_right(lines.head, " "), ' ');
    String type = fields.head;

    if(string_equals(create_string("v"), type)) model.positionCount++;
    else if(string_equals(create_string("vt"), type)) model.uvCount++;
    else if(string_equals(create_string("vn"), type)) model.normalCount++;
    else if(string_equals(create_string("f"), type)) model.faceCount++;
  }
  model.positions = arena_alloc_array(arena, vec3, model.positionCount);
  model.normals = arena_alloc_array(arena, vec3, model.normalCount);
  model.uvs = arena_alloc_array(arena, vec2, model.uvCount);
  model.positionIndices = arena_alloc_array(arena, ivec3, model.faceCount);
  model.normalIndices = arena_alloc_array(arena, ivec3, model.faceCount);
  model.uvIndices = arena_alloc_array(arena, ivec3, model.faceCount);

  model.positionCount = model.uvCount = model.normalCount = model.faceCount = 0;
  lines.tail = objSource;
  while(lines.tail.len) {
    lines = string_cut(lines.tail, '\n');
    Cut fields = string_cut(string_trim_right(lines.head, " "), ' ');
    String type = fields.head;
    String data = string_trim_left(fields.tail);
    
    if(string_equals(create_string("v"), type)) string_to_vec3(data, &model.positions[model.positionCount++]);
    else if(string_equals(create_string("vt"), type)) string_to_vec2(data, &model.uvs[model.uvCount++]);   
    else if(string_equals(create_string("vn"), type)) string_to_vec3(data, &model.normals[model.normalCount++]);   
    else if(string_equals(create_string("f"), type))  {
      string_to_face(
        data,
        model.positionCount,
        model.normalCount,
        model.uvCount,
        &model.positionIndices[model.faceCount],
        &model.normalIndices[model.faceCount],
        &model.uvIndices[model.faceCount]
      ); 
      model.faceCount++;
    }
  }
  return model;
}

#include <glad/glad.h>
#include <cJSON.h>

#include <stdio.h>
#include <string.h>

#include "scene_define.c"

#include "data_types/arena.c"
#include "opengl_utils.c"
#include "mesh.c"
#include "cglm/mat4.h"

/* 
 * Base 64 decoder
*/
const int base64invs[80] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };

int base64_isvalidchar(char c) {
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c == '+' || c == '/' || c == '=')
		return 1;
	return 0;
}

int base64_decode(String string, unsigned char *out) {
	size_t i;
	size_t j;
	int    v;

	for (i=0; i<string.len; i++) {
		if (!base64_isvalidchar(string.data[i])) {
			return 0;
		}
	}

	for (i=0, j=0; i<string.len; i+=4, j+=3) {
		v = base64invs[string.data[i]-43];
		v = (v << 6) | base64invs[string.data[i+1]-43];
		v = string.data[i+2]=='=' ? v << 6 : (v << 6) | base64invs[string.data[i+2]-43];
		v = string.data[i+3]=='=' ? v << 6 : (v << 6) | base64invs[string.data[i+3]-43];

		out[j] = (v >> 16) & 0xFF;
		if (string.data[i+2] != '=')
			out[j+1] = (v >> 8) & 0xFF;
		if (string.data[i+3] != '=')
			out[j+2] = v & 0xFF;
	}

	return 1;
}

//----------------------------
//Parsing
//----------------------------
//
typedef unsigned char byte;
typedef struct {
  byte* data;
  size_t len;
} Bytes;

uint64_t mesh_hash(int value) {
  return value;
}

bool int_is_equal(int a, int b) { return a == b; }

DEFINE_ARRAY(Bytes)
DEFINE_ARRAY(GLuint)

typedef struct {
  byte* data;
  size_t bytesPerElement;
  size_t stride;
  size_t count;
} GLTFAttribute;

GLTFAttribute load_attribute_from_gltf( Arena* arena, const Array(Bytes)* bufferArray, const cJSON* json, const cJSON* attribute, GLenum expectedComponentType, uint8_t expectedVecType) {
  const cJSON* accessors = cJSON_GetObjectItemCaseSensitive(json, "accessors");
  const cJSON* bufferViews = cJSON_GetObjectItemCaseSensitive(json, "bufferViews");

  const cJSON* accessor = cJSON_GetArrayItem(accessors, attribute->valueint);

  //accesors
  const cJSON* buffViewIndex = cJSON_GetObjectItemCaseSensitive(accessor, "bufferView");
  const cJSON* componentType = cJSON_GetObjectItemCaseSensitive(accessor, "componentType");
  const cJSON* count = cJSON_GetObjectItemCaseSensitive(accessor, "count");
  const cJSON* type = cJSON_GetObjectItemCaseSensitive(accessor, "type");
  const cJSON* accessorOffset = cJSON_GetObjectItemCaseSensitive(accessor, "byteOffset");
  size_t accessorOffsetValue = accessorOffset ? accessorOffset->valueint : 0;

  //bufferView
  const cJSON* buffView = cJSON_GetArrayItem(bufferViews, buffViewIndex->valueint);
  const cJSON* bufferIndex = cJSON_GetObjectItemCaseSensitive(buffView, "buffer");
  const cJSON* byteLength = cJSON_GetObjectItemCaseSensitive(buffView, "byteLength");
  const cJSON* bufferOffset = cJSON_GetObjectItemCaseSensitive(buffView, "byteOffset");
  const cJSON* stride = cJSON_GetObjectItemCaseSensitive(buffView, "byteStride");
  size_t bufferOffsetValue = bufferOffset ? bufferOffset->valueint : 0;
  
  //putting data
  Bytes buffer = array_get(Bytes, bufferArray, bufferIndex->valueint);
  buffer.data += accessorOffsetValue + bufferOffsetValue;
  buffer.len = byteLength->valueint;

  String typeStr;
  typeStr.data = type->valuestring;
  typeStr.len = strlen(type->valuestring);

  if(componentType->valueint != expectedComponentType) {
    fprintf(stderr, "Component Type doesn't match");
    fflush(stderr);
  }

  uint8_t vecType = 0;
  if(string_equals(typeStr, create_string("SCALAR"))) vecType = 1;
  else if(string_equals(typeStr, create_string("VEC2"))) vecType = 2;
  else if(string_equals(typeStr, create_string("VEC3"))) vecType = 3;
  else if(string_equals(typeStr, create_string("VEC4"))) vecType = 4;

  if(vecType != expectedVecType) {
    fprintf(stderr, "Vector Type doesn't match");
    fflush(stderr);
  }

  size_t bytesPerElement = vecType;
  switch(expectedComponentType) {
    case GL_FLOAT:
      bytesPerElement *= sizeof(float);
      break;
    case GL_UNSIGNED_BYTE:
      bytesPerElement *= sizeof(unsigned char);
      break;
    case GL_UNSIGNED_SHORT:
      bytesPerElement *= sizeof(unsigned short);
      break;
    case GL_UNSIGNED_INT:
      bytesPerElement *= sizeof(unsigned int);
      break;
  }
  size_t strideValue = stride ? stride->valueint : bytesPerElement;
  return (GLTFAttribute){buffer.data, bytesPerElement, strideValue, count->valueint};
}

size_t get_attribute_size(const GLTFAttribute* attribute) {
  return attribute->bytesPerElement*attribute->count;
}

Mesh load_mesh_from_gltf(Arena* arena, const Array(Bytes)* bufferArray, const Array(GLuint)* imageArray, const cJSON* json, const cJSON* primitive) {
  const cJSON* accessors = cJSON_GetObjectItemCaseSensitive(json, "accessors");
  const cJSON* bufferViews = cJSON_GetObjectItemCaseSensitive(json, "bufferViews");
  const cJSON* attributes = cJSON_GetObjectItemCaseSensitive(primitive, "attributes");

  const cJSON* POSITION = cJSON_GetObjectItemCaseSensitive(attributes, "POSITION");
  const cJSON* NORMAL = cJSON_GetObjectItemCaseSensitive(attributes, "NORMAL");
  const cJSON* TEXCOORD_0 = cJSON_GetObjectItemCaseSensitive(attributes, "TEXCOORD_0");

  //We will assume that any gltf without a position is an invalid gltf
  assert(primitive);
  
  GLTFAttribute position = load_attribute_from_gltf(arena, bufferArray, json, POSITION, GL_FLOAT, 3);

  byte blank[1] = {0};
  GLTFAttribute normal = {blank, 0, 0, position.count};
  if(NORMAL) {
    normal = load_attribute_from_gltf(arena, bufferArray, json, NORMAL, GL_FLOAT, 3);
  }
  GLTFAttribute texCoord = {blank, 0, 0, position.count};
  if(TEXCOORD_0) {
    texCoord = load_attribute_from_gltf(arena, bufferArray, json, TEXCOORD_0, GL_FLOAT, 2);
  }

  assert(position.count == normal.count && normal.count == texCoord.count);
  size_t vertexCount = position.count;
  size_t dataSize = get_attribute_size(&position) + get_attribute_size(&normal) + get_attribute_size(&texCoord);

  byte* vertexData = (byte*)arena_alloc(arena, dataSize);

  size_t offset = 0;
  size_t positionOffset = 0;
  size_t normalOffset = 0;
  size_t texCoordOffset = 0;
  for(size_t i = 0; i < vertexCount; i++) {
    memcpy(vertexData+offset, position.data+positionOffset, position.bytesPerElement);
    positionOffset += position.stride;
    offset += position.bytesPerElement;

    memcpy(vertexData+offset, normal.data+normalOffset, normal.bytesPerElement);
    normalOffset += normal.stride;
    offset += normal.bytesPerElement;

    memcpy(vertexData+offset, texCoord.data+texCoordOffset, texCoord.bytesPerElement);
    texCoordOffset += texCoord.stride;
    offset += texCoord.bytesPerElement;
  }

  //index Buffer
  const cJSON* indices = cJSON_GetObjectItemCaseSensitive(primitive, "indices");
  const cJSON* indexAccessor = cJSON_GetArrayItem(accessors, indices->valueint);

  //accessor
  const cJSON* indexCount = cJSON_GetObjectItemCaseSensitive(indexAccessor, "count");
  const cJSON* indexBufferViewIndex = cJSON_GetObjectItemCaseSensitive(indexAccessor, "bufferView");
  const cJSON* indexAccOffset = cJSON_GetObjectItemCaseSensitive(indexAccessor, "byteOffset");
  const cJSON* componentType = cJSON_GetObjectItemCaseSensitive(indexAccessor, "componentType");
  const int indexAccOffsetValue = indexAccOffset ? indexAccOffset->valueint : 0;

  //buffer view
  const cJSON* indexBufferView = cJSON_GetArrayItem(bufferViews, indexBufferViewIndex->valueint);
  const cJSON* indexLen = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteLength");
  const cJSON* indexBuff = cJSON_GetObjectItemCaseSensitive(indexBufferView, "buffer");
  const cJSON* indexBuffOffset = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteOffset");
  const int indexBuffOffsetValue = indexBuffOffset ? indexBuffOffset->valueint : 0;
  
  Bytes indexBytes = array_get(Bytes, bufferArray, indexBuff->valueint);
  indexBytes.data += indexBuffOffsetValue + indexAccOffsetValue;
  indexBytes.len = indexLen->valueint;

  size_t indexCountValue = indexCount->valueint;

  uint32_t* indexData = arena_alloc_array(arena, uint32_t, indexCountValue);
  switch(componentType->valueint) {
    case GL_UNSIGNED_BYTE: {
      unsigned char* indexList = (unsigned char*)indexBytes.data;
      for(size_t i = 0; i < indexCountValue; i++) {
        indexData[i] = indexList[i];
      }
    }
      break;
    case GL_UNSIGNED_SHORT: {
      unsigned short* indexList = (unsigned short*)indexBytes.data;
      for(size_t i = 0; i < indexCountValue; i++) {
        indexData[i] = indexList[i];
      }
    }
      break;
    case GL_UNSIGNED_INT: {
      unsigned int* indexList = (unsigned int*)indexBytes.data;
      for(size_t i = 0; i < indexCountValue; i++) {
        indexData[i] = indexList[i];
      }
    }
      break;
    default:
      fprintf(stderr, "NOT supported index type");
      fflush(stderr);
      break;
  }
  
  Attribute* attributeData = arena_alloc_array(arena, Attribute, 3);
  Array(Attribute) attributeList = create_array(Attribute, attributeData, 3);
  Attribute positonAttrib = (Attribute){GL_FLOAT, 3};
  Attribute normalAttrib = (Attribute){GL_FLOAT, 3};
  Attribute texCoordAttrib = (Attribute){GL_FLOAT, 2};
  array_set(Attribute, &attributeList, 0, positonAttrib);
  array_set(Attribute, &attributeList, 1, normalAttrib);
  array_set(Attribute, &attributeList, 2, texCoordAttrib);
  
  Geometry geometry = generate_geometry(vertexData, vertexCount, &attributeList, indexData, indexCountValue);
  const cJSON* materials = cJSON_GetObjectItemCaseSensitive(json, "materials");
  const cJSON* textures = cJSON_GetObjectItemCaseSensitive(json, "textures");

  Material material = (Material)DEFAULT_MATERIAL;
  //materials
  cJSON* materialIndex = cJSON_GetObjectItemCaseSensitive(primitive, "material");
  if(!materialIndex) return (Mesh){geometry, material};
  cJSON* materialJson = cJSON_GetArrayItem(materials, materialIndex->valueint);
  
  cJSON* pbr = cJSON_GetObjectItemCaseSensitive(materialJson, "pbrMetallicRoughness");
  if(pbr) {
    cJSON* colorFactor  = cJSON_GetObjectItemCaseSensitive(pbr, "baseColorFactor");
    cJSON* colorTexture = cJSON_GetObjectItemCaseSensitive(pbr, "baseColorTexture");
    cJSON* metallicRoughnessTexture = cJSON_GetObjectItemCaseSensitive(pbr, "metallicRoughnessTexture");
    cJSON* metallicFactor = cJSON_GetObjectItemCaseSensitive(pbr, "metallicFactor");
    cJSON* roughnessFactor = cJSON_GetObjectItemCaseSensitive(pbr, "rounghnessFactor");
  
    vec4 defaultColorFactor = {1.0, 1.0, 1.0, 1.0};
    glm_vec4_copy(material.colorFactor, defaultColorFactor);
    if(colorFactor) {
      material.colorFactor[0] = cJSON_GetArrayItem(colorFactor, 0)->valuedouble; 
      material.colorFactor[1] = cJSON_GetArrayItem(colorFactor, 1)->valuedouble; 
      material.colorFactor[2] = cJSON_GetArrayItem(colorFactor, 2)->valuedouble; 
    }
    if(colorTexture) {
      cJSON* index = cJSON_GetObjectItemCaseSensitive(colorTexture, "index");
      cJSON* texture = cJSON_GetArrayItem(textures, index->valueint);
      cJSON* source = cJSON_GetObjectItemCaseSensitive(texture, "source");
      material.colorTexture = array_get(GLuint, imageArray, source->valueint);
    }
    if(metallicRoughnessTexture) {
      cJSON* index = cJSON_GetObjectItemCaseSensitive(metallicRoughnessTexture, "index");
      cJSON* texture = cJSON_GetArrayItem(textures, index->valueint);
      cJSON* source = cJSON_GetObjectItemCaseSensitive(texture, "source");
      material.metallicRoughnessTexture = array_get(GLuint, imageArray, source->valueint);
    }
    material.metallicFactor = 1.0;
    if(metallicFactor) {
      material.metallicFactor = metallicFactor->valuedouble;
    }
    material.roughnessFactor = 1.0;
    if(roughnessFactor) {
      material.roughnessFactor = roughnessFactor->valuedouble;
    }
  }
  cJSON* normalTexture = cJSON_GetObjectItemCaseSensitive(materialJson, "normalTexture");
  if(normalTexture) {
    cJSON* index = cJSON_GetObjectItemCaseSensitive(normalTexture, "index");
    cJSON* texture = cJSON_GetArrayItem(textures, index->valueint);
    cJSON* source = cJSON_GetObjectItemCaseSensitive(texture, "source");
    material.normalTexture = array_get(GLuint, imageArray, source->valueint);
  }
  cJSON* occulsionTexture = cJSON_GetObjectItemCaseSensitive(materialJson, "occulsionTexture");
  if(occulsionTexture) {
    cJSON* index = cJSON_GetObjectItemCaseSensitive(occulsionTexture, "index");
    cJSON* texture = cJSON_GetArrayItem(textures, index->valueint);
    cJSON* source = cJSON_GetObjectItemCaseSensitive(texture, "source");
    material.occulsionTexture = array_get(GLuint, imageArray, source->valueint);
  }
  cJSON* emissiveTexture = cJSON_GetObjectItemCaseSensitive(materialJson, "emissiveTexture");
  if(emissiveTexture) {
    cJSON* index = cJSON_GetObjectItemCaseSensitive(emissiveTexture, "index");
    cJSON* texture = cJSON_GetArrayItem(textures, index->valueint);
    cJSON* source = cJSON_GetObjectItemCaseSensitive(texture, "source");
    material.emissiveTexture = array_get(GLuint, imageArray, source->valueint);
  }

  cJSON* emissiveFactor = cJSON_GetObjectItemCaseSensitive(materialJson, "emissiveFactor");

  vec3 defaultEmissiveFactor = {1.0, 1.0, 1.0};
  glm_vec3_copy(material.emissiveFactor, defaultEmissiveFactor);
  if(emissiveFactor) {
    material.emissiveFactor[0] = cJSON_GetArrayItem(emissiveFactor, 0)->valuedouble; 
    material.emissiveFactor[1] = cJSON_GetArrayItem(emissiveFactor, 1)->valuedouble; 
    material.emissiveFactor[2] = cJSON_GetArrayItem(emissiveFactor, 2)->valuedouble; 
  }
  return (Mesh){geometry, material, GLM_MAT4_IDENTITY_INIT};
}

#include "data_types/array.c"

Array(Mesh) extract_meshes_from_gltf(Arena* arena, String filePath) {
  Ccreate_string(filePath, cFilePath);
  String source = read_file(arena, cFilePath);
  Ccreate_string(source, cSource)
  cJSON *json = cJSON_Parse(cSource);
  assert(json);

  const cJSON* meshes = cJSON_GetObjectItemCaseSensitive(json, "meshes");
  const cJSON* buffers = cJSON_GetObjectItemCaseSensitive(json, "buffers");
  const cJSON* images = cJSON_GetObjectItemCaseSensitive(json, "images");

  size_t bufferCount = cJSON_GetArraySize(buffers);
  Bytes* bufferData = arena_alloc_array(arena, Bytes, bufferCount);
  Array(Bytes) bufferArray = create_array(Bytes, bufferData, bufferCount);

  size_t imageCount = images ? cJSON_GetArraySize(images) : 0;
  GLuint* imageData = arena_alloc_array(arena, GLuint, imageCount);
  Array(GLuint) imageArray = create_array(GLuint, imageData, imageCount);

  //buffer handling
  for(int i = 0; i < bufferCount; i++) {
    const cJSON* buffer = cJSON_GetArrayItem(buffers, i);
    const cJSON* buffLen = cJSON_GetObjectItemCaseSensitive(buffer, "byteLength");
    const cJSON* buffURI = cJSON_GetObjectItemCaseSensitive(buffer, "uri");

    String uri;
    uri.len = strlen(buffURI->valuestring);
    uri.data = (const char*)buffURI->valuestring;
    byte* data = arena_alloc_array(arena, byte, buffLen->valueint);
    
    Bytes bytes = (Bytes){data, buffLen->valueint};
    if(string_contains(uri, create_string("data:"))) {
      Cut cut = string_cut(uri, ';');
      cut = string_cut(cut.tail, ',');
      int base = string_to_int(string_span(cut.head.data+4, cut.head.data+cut.head.len));
      if(base == 64) base64_decode(cut.tail, data);
      else{ 
        fprintf(stderr, "%d is an invalid base!", base);
      }
    }
    else {
      int32_t index = string_find_reverse(filePath, '/');
      String parentPath;
      if(index == -1) parentPath = filePath; 
      //+1 is there to include the '/'
      else parentPath = string_span(filePath.data, filePath.data+index+1);
      String resFile = (String){0};
      resFile.data = buffURI->valuestring;
      resFile.len = strlen(buffURI->valuestring);

      String binFilePath = string_concatenate(arena, parentPath, resFile);
      Ccreate_string(binFilePath, cBinFile);
      
      FILE* file = fopen(cBinFile, "rb");
      if(!file) fprintf(stderr, "There is no binary file %s", cBinFile);
      fread(data, buffLen->valueint, 1, file);
      fclose(file);
    }
    array_set(Bytes, &bufferArray, i, bytes);
  }

  //buffer handling
  for(int i = 0; i < imageCount; i++) {
    const cJSON* image = cJSON_GetArrayItem(images, i);
    const cJSON* imageURI = cJSON_GetObjectItemCaseSensitive(image, "uri");

    String uri;
    uri.len = strlen(imageURI->valuestring);
    uri.data = (const char*)imageURI->valuestring;
  
    int32_t index = string_find_reverse(filePath, '/');
    String parentPath;
    if(index == -1) parentPath = filePath; 
    //+1 is there to include the '/'
    else parentPath = string_span(filePath.data, filePath.data+index+1);
    String resFile = (String){0};
    resFile.data = imageURI->valuestring;
    resFile.len = strlen(imageURI->valuestring);

    String binFilePath = string_concatenate(arena, parentPath, resFile);
    Ccreate_string(binFilePath, cBinFile);
    
    GLuint texture = create_texture(cBinFile);
    
    array_set(GLuint, &imageArray, i, texture);
  }
  
  size_t meshCount = meshes ? cJSON_GetArraySize(meshes) : 0;
  Mesh* meshData = arena_alloc_array(arena, Mesh, meshCount);
  Array(Mesh) result = create_array(Mesh, meshData, meshCount);
  //Adding Models
  for(int i = 0; i < meshCount; i++) {
    const cJSON* mesh = cJSON_GetArrayItem(meshes, i);
    const cJSON* primitives = cJSON_GetObjectItemCaseSensitive(mesh, "primitives");
    //Adding Meshes
    for(int j = 0; j < cJSON_GetArraySize(primitives); j++) {
      const cJSON* prim = cJSON_GetArrayItem(primitives, j);
      array_set(Mesh, &result, i, load_mesh_from_gltf(arena, &bufferArray, &imageArray, json, prim));
    }
  }

  cJSON_Delete(json);
  return result;
}

/*
Scene load_scene_from_gltf(Arena* arena, String filePath) {
  Ccreate_string(filePath, cFilePath);
  String source = read_file(arena, cFilePath);
  Ccreate_string(source, cSource)
  cJSON *json = cJSON_Parse(cSource);
  assert(json);

  const cJSON* meshes = cJSON_GetObjectItemCaseSensitive(json, "meshes");
  const cJSON* buffers = cJSON_GetObjectItemCaseSensitive(json, "buffers");
  const cJSON* images = cJSON_GetObjectItemCaseSensitive(json, "images");

  size_t bufferCount = cJSON_GetArraySize(buffers);
  Bytes* bufferData = arena_alloc_array(arena, Bytes, bufferCount);
  Array(Bytes) bufferArray = create_array(Bytes, bufferData, bufferCount);

  size_t imageCount = images ? cJSON_GetArraySize(images) : 0;
  GLuint* imageData = arena_alloc_array(arena, GLuint, imageCount);
  Array(GLuint) imageArray = create_array(GLuint, imageData, imageCount);

  //buffer handling
  for(int i = 0; i < bufferCount; i++) {
    const cJSON* buffer = cJSON_GetArrayItem(buffers, i);
    const cJSON* buffLen = cJSON_GetObjectItemCaseSensitive(buffer, "byteLength");
    const cJSON* buffURI = cJSON_GetObjectItemCaseSensitive(buffer, "uri");

    String uri;
    uri.len = strlen(buffURI->valuestring);
    uri.data = (const char*)buffURI->valuestring;
    byte* data = arena_alloc_array(arena, byte, buffLen->valueint);
    
    Bytes bytes = (Bytes){data, buffLen->valueint};
    if(string_contains(uri, create_string("data:"))) {
      Cut cut = string_cut(uri, ';');
      cut = string_cut(cut.tail, ',');
      int base = string_to_int(string_span(cut.head.data+4, cut.head.data+cut.head.len));
      if(base == 64) base64_decode(cut.tail, data);
      else{ 
        fprintf(stderr, "%d is an invalid base!", base);
      }
    }
    else {
      int32_t index = string_find_reverse(filePath, '/');
      String parentPath;
      if(index == -1) parentPath = filePath; 
      //+1 is there to include the '/'
      else parentPath = string_span(filePath.data, filePath.data+index+1);
      String resFile = (String){0};
      resFile.data = buffURI->valuestring;
      resFile.len = strlen(buffURI->valuestring);

      String binFilePath = string_concatenate(arena, parentPath, resFile);
      Ccreate_string(binFilePath, cBinFile);
      
      FILE* file = fopen(cBinFile, "rb");
      if(!file) fprintf(stderr, "There is no binary file %s", cBinFile);
      fread(data, buffLen->valueint, 1, file);
      fclose(file);
    }
    array_set(Bytes, &bufferArray, i, bytes);
  }

  //buffer handling
  for(int i = 0; i < imageCount; i++) {
    const cJSON* image = cJSON_GetArrayItem(images, i);
    const cJSON* imageURI = cJSON_GetObjectItemCaseSensitive(image, "uri");

    String uri;
    uri.len = strlen(imageURI->valuestring);
    uri.data = (const char*)imageURI->valuestring;
  
    int32_t index = string_find_reverse(filePath, '/');
    String parentPath;
    if(index == -1) parentPath = filePath; 
    //+1 is there to include the '/'
    else parentPath = string_span(filePath.data, filePath.data+index+1);
    String resFile = (String){0};
    resFile.data = imageURI->valuestring;
    resFile.len = strlen(imageURI->valuestring);

    String binFilePath = string_concatenate(arena, parentPath, resFile);
    Ccreate_string(binFilePath, cBinFile);
    
    GLuint texture = create_texture(cBinFile);
    
    array_set(GLuint, &imageArray, i, texture);
  }
  
  size_t meshCount = meshes ? cJSON_GetArraySize(meshes) : 0;
  Mesh* meshData = arena_alloc_array(arena, Mesh, meshCount);
  Array(Mesh) result = create_array(Mesh, meshData, meshCount);
  //Adding Models
  for(int i = 0; i < meshCount; i++) {
    const cJSON* mesh = cJSON_GetArrayItem(meshes, i);
    const cJSON* primitives = cJSON_GetObjectItemCaseSensitive(mesh, "primitives");
    //Adding Meshes
    for(int j = 0; j < cJSON_GetArraySize(primitives); j++) {
      const cJSON* prim = cJSON_GetArrayItem(primitives, j);
      array_set(Mesh, &result, i, load_mesh_from_gltf(arena, &bufferArray, &imageArray, json, prim));
    }
  }

  cJSON_Delete(json);
  return result;
}
*/

#endif
