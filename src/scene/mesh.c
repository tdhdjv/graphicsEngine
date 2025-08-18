#ifndef MESH_HEADER
#define MESH_HEADER

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cglm/util.h>
#include <cglm/vec3.h>

#include <stdint.h>
#include <stdio.h>

#include "../data_types/array.c"

#include "scene_defines.c"

DEFINE_ARRAY(Attribute)
Geometry generate_geometry(void* vertexData, size_t vertexCount, const Array(Attribute)* attributes, uint32_t *indexData, size_t indexCount) {
  //pre calculations
  size_t stride = 0;
  for(size_t i = 0; i < attributes->capacity; i++) {
    Attribute attribute = array_get(Attribute, attributes, i);
    if(attribute.type == GL_FLOAT) stride += sizeof(float)*attribute.vec_dimension;
    if(attribute.type == GL_UNSIGNED_INT) stride += sizeof(unsigned)*attribute.vec_dimension;
    if(attribute.type == GL_UNSIGNED_SHORT) stride += sizeof(unsigned short)*attribute.vec_dimension;
    if(attribute.type == GL_INT) stride += sizeof(int)*attribute.vec_dimension;
    if(attribute.type == GL_SHORT) stride += sizeof(short)*attribute.vec_dimension;
  };
    
  GLuint VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, stride*vertexCount, vertexData,GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indexCount,indexData, GL_STATIC_DRAW);

  size_t offset = 0;
  for(size_t i = 0; i < attributes->capacity; i++) {
    Attribute attribute = array_get(Attribute, attributes, i);
    glVertexAttribPointer(i, attribute.vec_dimension, attribute.type, GL_FALSE, stride, (void *)offset);
    if(attribute.type == GL_FLOAT) offset += sizeof(float)*attribute.vec_dimension;
    if(attribute.type == GL_UNSIGNED_INT) offset += sizeof(unsigned)*attribute.vec_dimension;
    if(attribute.type == GL_UNSIGNED_SHORT) offset += sizeof(unsigned short)*attribute.vec_dimension;
    if(attribute.type == GL_INT) offset += sizeof(int)*attribute.vec_dimension;
    if(attribute.type == GL_SHORT) offset += sizeof(short)*attribute.vec_dimension;
    glEnableVertexAttribArray(i);
  }

  return (Geometry){VAO, VBO, EBO, indexCount};
}

Geometry generate_quad_geometry(Arena* arena, vec3 horizontal, vec3 vertical, uint32_t subDivision) {
  size_t vertexCount = (subDivision + 2) * (subDivision + 2);
  size_t indexCount = 6 * (subDivision + 1) * (subDivision + 1);
  float vertexData[vertexCount * 8];
  uint32_t indexData[indexCount];

  vec3 normal;
  glm_cross(horizontal, vertical, normal);
  glm_normalize(normal);

  uint32_t n = 0;

  for (uint32_t i = 0; i < subDivision + 2; i++) {
    for (uint32_t j = 0; j < subDivision + 2; j++) {
      float u = ((float)i) / (float)(subDivision + 1) - 0.5f;
      float v = ((float)j) / (float)(subDivision + 1) - 0.5f;
      vertexData[n++] = horizontal[0] * u + vertical[0] * v;
      vertexData[n++] = horizontal[1] * u + vertical[1] * v;
      vertexData[n++] = horizontal[2] * u + vertical[2] * v;

      vertexData[n++] = normal[0];
      vertexData[n++] = normal[1];
      vertexData[n++] = normal[2];

      vertexData[n++] = u;
      vertexData[n++] = v;
    }
  }

  n = 0;

  for (uint32_t i = 0; i < subDivision + 1; i++) {
    for (uint32_t j = 0; j < subDivision + 1; j++) {
      size_t firstIndex = (j + i * (subDivision + 2));
      indexData[n++] = firstIndex;
      indexData[n++] = firstIndex + (subDivision + 2);
      indexData[n++] = firstIndex + 1;
      indexData[n++] = firstIndex + 1;
      indexData[n++] = firstIndex + (subDivision + 2);
      indexData[n++] = firstIndex + (subDivision + 2) + 1;
    }
  }
  Attribute* attributeData = arena_alloc_array(arena, Attribute, 3);
  Array(Attribute) attributes = create_array(Attribute, attributeData, 3);
  Attribute positonAttrib = (Attribute){GL_FLOAT, 3};
  Attribute normalAttrib = (Attribute){GL_FLOAT, 3};
  Attribute texCoordAttrib = (Attribute){GL_FLOAT, 2};
  array_set(Attribute, &attributes, 0, positonAttrib);
  array_set(Attribute, &attributes, 1, normalAttrib);
  array_set(Attribute, &attributes, 2, texCoordAttrib);

  return generate_geometry(vertexData, vertexCount, &attributes, indexData, indexCount);
}

// Math behind this is explained in /documentation/icosphere.md
Geometry generate_icosphere_geometry(Arena* arena, uint32_t subDivision) {
  size_t vertexCount = 10 * (subDivision + 1) * (subDivision + 1) + 2;
  size_t indexCount = 60 * (subDivision + 1) * (subDivision + 1);

  Geometry result = {0};

  ArenaMark mark = create_arena_mark(arena);
  float* vertexData = arena_alloc_array(arena, float, vertexCount * 6);
  uint32_t* indexData = arena_alloc_array(arena, int, indexCount);

  float phi = 0.5 * (1.0 + sqrt(5.0));
  vec3 northPole = {0.0, 1.0, phi};
  vec3 southPole = {0.0, -1.0, -phi};
  vec3 strip1[4] = {
      {phi, 0.0, 1.0}, {1.0, phi, 0.0}, {phi, 0.0, -1.0}, {0.0, 1.0, -phi}};
  vec3 strip2[4] = {
      {1.0, phi, 0.0}, {-1.0, phi, 0.0}, {0.0, 1.0, -phi}, {-phi, 0.0, -1.0}};
  vec3 strip3[4] = {
      {-1.0, phi, 0.0}, {-phi, 0.0, 1.0}, {-phi, 0.0, -1.0}, {-1.0, -phi, 0.0}};
  vec3 strip4[4] = {
      {-phi, 0.0, 1.0}, {0.0, -1.0, phi}, {-1.0, -phi, 0.0}, {1.0, -phi, 0.0}};
  vec3 strip5[4] = {
      {0.0, -1.0, phi}, {phi, 0.0, 1.0}, {1.0, -phi, 0.0}, {phi, 0.0, -1.0}};

  vec3 *strips[5] = {strip1, strip2, strip3, strip4, strip5};

  size_t n = 0;
  vec3 normalizedNorth;
  glm_normalize_to(northPole, normalizedNorth);
  //position
  vertexData[n++] = normalizedNorth[0];
  vertexData[n++] = normalizedNorth[1];
  vertexData[n++] = normalizedNorth[2];
  
  //normal
  vertexData[n++] = normalizedNorth[0];
  vertexData[n++] = normalizedNorth[1];
  vertexData[n++] = normalizedNorth[2];

  for (int i = 0; i < 5; i++) {
    vec3 *strip = strips[i];
    for (uint32_t j = 0; j < subDivision + 1; j++) {
      for (uint32_t k = 0; k < 2 * (subDivision + 1); k++) {
        vec3 vertex;
        glm_vec3_zero(vertex);

        float u = (float)(k) / (float)(subDivision + 1);
        float v = (float)(j) / (float)(subDivision + 1);

        vec3 U, V, W;

        if (v >= u) {
          v = 1.0 - v;
          glm_vec3_copy(strip[1], U);
          glm_vec3_copy(strip[0], V);
          glm_vec3_copy(northPole, W);
        } else if (u <= 1.0) {
          u = 1.0 - u;
          glm_vec3_copy(strip[0], U);
          glm_vec3_copy(strip[1], V);
          glm_vec3_copy(strip[2], W);
        } else if (u <= v + 1.0) {
          u = u - 1.0;
          v = 1.0 - v;
          glm_vec3_copy(strip[3], U);
          glm_vec3_copy(strip[2], V);
          glm_vec3_copy(strip[1], W);
        } else {
          u = 2.0 - u;
          glm_vec3_copy(strip[2], U);
          glm_vec3_copy(strip[3], V);
          glm_vec3_copy(southPole, W);
        }

        float w = 1.0 - u - v;

        glm_vec3_scale(U, u, U);
        glm_vec3_scale(V, v, V);
        glm_vec3_scale(W, w, W);

        glm_vec3_add(vertex, U, vertex);
        glm_vec3_add(vertex, V, vertex);
        glm_vec3_add(vertex, W, vertex);

        glm_normalize(vertex);

        //position
        vertexData[n++] = vertex[0];
        vertexData[n++] = vertex[1];
        vertexData[n++] = vertex[2];
        
        //normal
        vertexData[n++] = vertex[0];
        vertexData[n++] = vertex[1];
        vertexData[n++] = vertex[2];
      }
    }
  }

  glm_normalize(southPole);
  //position
  vertexData[n++] = southPole[0];
  vertexData[n++] = southPole[1];
  vertexData[n++] = southPole[2];

  //normal
  vertexData[n++] = southPole[0];
  vertexData[n++] = southPole[1];
  vertexData[n++] = southPole[2];
  
  n = 0;
  size_t vertexPerRow = 2 * (subDivision + 1);
  size_t vertexPerStrip = vertexPerRow * (subDivision + 1);

  for (int i = 0; i < 5; i++) {
    for (uint32_t j = 0; j < subDivision; j++) {
      for (uint32_t k = 0; k < 2 * subDivision + 1; k++) {
        uint32_t firstIndex = k + vertexPerRow * j + vertexPerStrip * i + 1;

        indexData[n++] = firstIndex;
        indexData[n++] = firstIndex + 1;
        indexData[n++] = firstIndex + vertexPerRow + 1;

        indexData[n++] = firstIndex;
        indexData[n++] = firstIndex + vertexPerRow + 1;
        indexData[n++] = firstIndex + vertexPerRow;
      }
    }

    for (uint32_t k = 0; k < subDivision + 1; k++) {
      uint32_t firstIndex = k + vertexPerStrip * (i + 1) - vertexPerRow + 1;
      uint32_t secondIndex = (subDivision + 1 - k) * vertexPerRow +
                             vertexPerStrip * ((i + 1) % 5) + 1;
      uint32_t thirdIndex = secondIndex - vertexPerRow;
      uint32_t forthIndex = firstIndex + 1;

      if (k == 0)
        secondIndex = 0;

      indexData[n++] = firstIndex;
      indexData[n++] = forthIndex;
      indexData[n++] = thirdIndex;

      indexData[n++] = firstIndex;
      indexData[n++] = thirdIndex;
      indexData[n++] = secondIndex;
    }

    for (uint32_t k = 0; k < subDivision; k++) {
      uint32_t firstIndex =
          subDivision + 1 + k + vertexPerStrip * (i + 1) - vertexPerRow + 1;
      uint32_t secondIndex = k + vertexPerStrip * ((i + 1) % 5) + 1;
      uint32_t thirdIndex = secondIndex + 1;
      uint32_t forthIndex = firstIndex + 1;

      indexData[n++] = firstIndex;
      indexData[n++] = forthIndex;
      indexData[n++] = thirdIndex;

      indexData[n++] = firstIndex;
      indexData[n++] = thirdIndex;
      indexData[n++] = secondIndex;
    }

    for (uint32_t k = 0; k < subDivision + 1; k++) {
      uint32_t firstIndex = vertexPerRow * (k + 1) + vertexPerStrip * i;
      uint32_t secondIndex = firstIndex + vertexPerRow;
      uint32_t thirdIndex = vertexPerRow - k + vertexPerStrip * ((i + 1) % 5);
      uint32_t forthIndex = thirdIndex + 1;

      if (k == subDivision)
        secondIndex = thirdIndex - 1;
      if (k == 0)
        forthIndex = vertexCount - 1;

      indexData[n++] = firstIndex;
      indexData[n++] = forthIndex;
      indexData[n++] = thirdIndex;

      indexData[n++] = firstIndex;
      indexData[n++] = thirdIndex;
      indexData[n++] = secondIndex;
    }
  }
  Attribute* attributeData = arena_alloc_array(arena, Attribute, 2);
  Array(Attribute) attributes = create_array(Attribute, attributeData, 2);
  Attribute positonAttrib = (Attribute){GL_FLOAT, 3};
  Attribute normalAttrib = (Attribute){GL_FLOAT, 3};
  array_set(Attribute, &attributes, 0, positonAttrib);
  array_set(Attribute, &attributes, 1, normalAttrib);

  result = generate_geometry(vertexData, vertexCount, &attributes, indexData, indexCount);

  arena_return_to_mark(&mark);

  return result;
}
/*
//------------------------------------------
// OBJ FILE
//------------------------------------------

#include "../data_types/string.c"

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
    Cut fields = string_cut(string_trim_right(lines.head), ' ');
    String type = fields.head;

    if(string_equals(STRING("v"), type)) model.positionCount++;
    else if(string_equals(STRING("vt"), type)) model.uvCount++;
    else if(string_equals(STRING("vn"), type)) model.normalCount++;
    else if(string_equals(STRING("f"), type)) model.faceCount++;
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
    Cut fields = string_cut(string_trim_right(lines.head), ' ');
    String type = fields.head;
    String data = string_trim_left(fields.tail);
    
    if(string_equals(STRING("v"), type)) string_to_vec3(data, &model.positions[model.positionCount++]);
    else if(string_equals(STRING("vt"), type)) string_to_vec2(data, &model.uvs[model.uvCount++]);   
    else if(string_equals(STRING("vn"), type)) string_to_vec3(data, &model.normals[model.normalCount++]);   
    else if(string_equals(STRING("f"), type))  {
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

#include "../data_types/io.c"
#include "../data_types/hashtable.c"

typedef struct{
  uint32_t positionIndex;
  uint32_t normalIndex;
  uint32_t uvIndex;
} PosNormTex;

bool is_equal(PosNormTex a, PosNormTex b) {
  return a.positionIndex == b.positionIndex && a.normalIndex == b.normalIndex && a.uvIndex == b.uvIndex;
}

uint64_t hash(PosNormTex key) {
  uint64_t hash;
  hash = key.positionIndex;
  hash <<= 21;
  hash = key.normalIndex;
  hash <<= 21;
  hash = key.uvIndex;
  return hash;
}

DEFINE_HASH_TABLE(PosNormTex, uint32_t, hash, is_equal);

Mesh gen_obj_model(Arena* arena, const char* filename) {
  String objSource = read_file(arena, filename);
  ObjModel model = parse_obj(arena, objSource);
  Mesh result = {0};

  ArenaMark mark = create_arena_mark(arena);

  KeyValue(PosNormTex, uint32_t)* data = arena_alloc_array(arena, KeyValue(PosNormTex, uint32_t), 3*model.faceCount);
  HashTable(PosNormTex, uint32_t) table = create_hash_table(PosNormTex, uint32_t, data, 3*model.faceCount);

  size_t vertexCount = 0;
  const size_t indexCount = 3*model.faceCount;

  //count
  //for every face
  for(size_t i = 0; i < model.faceCount; i++) {
    //for every vertex per face
    for(size_t j = 0; j < 3; j++) {
      PosNormTex pnt = (PosNormTex){model.positionIndices[i][j], model.normalIndices[i][j], model.uvIndices[i][j]};
      if(hash_table_contains(PosNormTex, uint32_t, &table, pnt)) continue;
      hash_table_set(PosNormTex, uint32_t, &table, pnt, vertexCount++);
    }
  }

  float* vertexData = arena_alloc_array(arena, float, FLOATS_PER_VERTEX*vertexCount);
  uint32_t floatIndex = 0;
  uint32_t vertexIndex = 0;

  //define the vertices
  for(size_t i = 0; i < model.faceCount; i++) {
    for(size_t j = 0; j < 3; j++) {
      PosNormTex pnt = (PosNormTex){model.positionIndices[i][j], model.normalIndices[i][j], model.uvIndices[i][j]};
      //check if this is duplicate vertex
      vertexIndex = floatIndex/FLOATS_PER_VERTEX;
      if(hash_table_get(PosNormTex, uint32_t, &table, pnt, 0) != vertexIndex) continue;
      
      //-1 is added since objs are 1 indexed for some reason
      //position
      vertexData[floatIndex++] = pnt.positionIndex ? model.positions[pnt.positionIndex-1][0] : 0;
      vertexData[floatIndex++] = pnt.positionIndex ? model.positions[pnt.positionIndex-1][1] : 0;
      vertexData[floatIndex++] = pnt.positionIndex ? model.positions[pnt.positionIndex-1][2] : 0;
      
      //normal
      vertexData[floatIndex++] = pnt.normalIndex ? model.normals[pnt.normalIndex-1][0] : 0;
      vertexData[floatIndex++] = pnt.normalIndex ? model.normals[pnt.normalIndex-1][1] : 0;
      vertexData[floatIndex++] = pnt.normalIndex ? model.normals[pnt.normalIndex-1][2] : 0;

      // uv
      vertexData[floatIndex++] = pnt.uvIndex ? model.uvs[pnt.uvIndex-1][0] : 0;
      vertexData[floatIndex++] = pnt.uvIndex ? model.uvs[pnt.uvIndex-1][1] : 0;
    }
  }

  uint32_t* indexData = arena_alloc_array(arena, uint32_t, 3*model.faceCount);
  uint32_t uintIndex = 0;

  //define indices 
  for(size_t i = 0; i < model.faceCount; i++) {
    for(size_t j = 0; j < 3; j++) {
      PosNormTex pnt = (PosNormTex){model.positionIndices[i][j], model.normalIndices[i][j], model.uvIndices[i][j]};
      uint32_t index = hash_table_get(PosNormTex, uint32_t, &table, pnt,0);
      //check if this is duplicate vertex
      indexData[uintIndex++] = index;
    }
  }
  
  result = gen_mesh(vertexData, indexData, vertexCount, indexCount);
  arena_return_to_mark(&mark);
  return result;
}

//---------------------------------------------------------------
* GLTF FILE FORMAT
//-----------------------------------------------------------------
#include <cJSON.h>
#include <stdio.h>
#include "../data_types/io.c"
#include "../data_types/arena.c"
#include "../data_types/hashtable.c"

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


typedef unsigned char byte;
typedef struct {
  byte* data;
  size_t len;
} Bytes;

uint64_t mesh_hash(int value) {
  return value;
}

bool int_is_equal(int a, int b) { return a == b; }

DEFINE_HASH_TABLE(int, Bytes, mesh_hash, int_is_equal)

typedef struct {
  float* data;
  uint8_t vec_type;
  size_t count;
} Attribute;


Attribute get_attribute(Arena* arena, HashTable(int, Bytes)* bufferTable, const cJSON* accessors, const cJSON* bufferViews, const cJSON* buffers, const cJSON* attribute) {

  const cJSON* accessor = cJSON_GetArrayItem(accessors, attribute->valueint);

  //accesors
  const cJSON* buffViewIndex = cJSON_GetObjectItemCaseSensitive(accessor, "bufferView");
  const cJSON* componentType = cJSON_GetObjectItemCaseSensitive(accessor, "componentType");
  const cJSON* count = cJSON_GetObjectItemCaseSensitive(accessor, "count");
  const cJSON* type = cJSON_GetObjectItemCaseSensitive(accessor, "type");
  const cJSON* accessorOffset = cJSON_GetObjectItemCaseSensitive(accessor, "byteOffset");
  size_t accessorOffsetValue = accessorOffset ? accessorOffset->valueint : 0;

  assert(componentType->valueint == GL_FLOAT);

  //bufferView
  const cJSON* buffView = cJSON_GetArrayItem(bufferViews, buffViewIndex->valueint);
  const cJSON* bufferIndex = cJSON_GetObjectItemCaseSensitive(buffView, "buffer");
  const cJSON* byteLength = cJSON_GetObjectItemCaseSensitive(buffView, "byteLength");
  const cJSON* bufferOffset = cJSON_GetObjectItemCaseSensitive(buffView, "byteOffset");
  const cJSON* stride = cJSON_GetObjectItemCaseSensitive(buffView, "byteStride");
  size_t bufferOffsetValue = bufferOffset ? bufferOffset->valueint : 0;
  
  //putting data
  Bytes buffer = hash_table_get(int, Bytes, bufferTable, bufferIndex->valueint, (Bytes){0});
  buffer.data += accessorOffsetValue + bufferOffsetValue;
  buffer.len = byteLength->valueint;

  String typeStr;
  typeStr.data = type->valuestring;
  typeStr.len = strlen(type->valuestring);

  uint8_t vec_type = 0;
  
  if(string_equals(typeStr, STRING("VEC4"))) vec_type = 4;
  else if(string_equals(typeStr, STRING("VEC3"))) vec_type = 3;
  else if(string_equals(typeStr, STRING("VEC2"))) vec_type = 2;
  else if(string_equals(typeStr, STRING("VEC1"))) vec_type = 1;

  float* data = arena_alloc_array(arena, float, count->valueint * vec_type);
  float* bufferData = (float*)buffer.data;
  size_t float_stride = stride ? stride->valueint/sizeof(float) : vec_type;

  for(size_t i = 0; i < count->valueint; i++) {
    for(size_t j = 0; j < vec_type; j++) {
      data[i*vec_type + j] = bufferData[i*float_stride + j];
    }
  }

  return (Attribute){data, vec_type, count->valueint};
}

Mesh gen_gltf(Arena* arena, String filePath) {
  CSTRING(filePath, cFilePath);
  String source = read_file(arena, cFilePath);
  CSTRING(source, cSource)
  cJSON *json = cJSON_Parse(cSource);
  assert(json);

  const cJSON* meshes = cJSON_GetObjectItemCaseSensitive(json, "meshes");
  const cJSON* accessors = cJSON_GetObjectItemCaseSensitive(json, "accessors");
  const cJSON* bufferViews = cJSON_GetObjectItemCaseSensitive(json, "bufferViews");
  const cJSON* buffers = cJSON_GetObjectItemCaseSensitive(json, "buffers");
  size_t bufferCount = cJSON_GetArraySize(buffers);

  Mesh result = (Mesh){0};

  ArenaMark mark = create_arena_mark(arena);

  KeyValue(int, Bytes)* data = arena_alloc_array(arena, KeyValue(int, Bytes), bufferCount);
  HashTable(int, Bytes) bufferTable = create_hash_table(int, Bytes, data, sizeof(KeyValue(int, Bytes))*bufferCount);
  
  //buffer handling
  for(int i = 0; i < bufferCount; i++) {
    const cJSON* buffer = cJSON_GetArrayItem(buffers, i);
    const cJSON* normalBuffLen = cJSON_GetObjectItemCaseSensitive(buffer, "byteLength");
    const cJSON* normalBuffURI = cJSON_GetObjectItemCaseSensitive(buffer, "uri");

    String uri;
    uri.len = strlen(normalBuffURI->valuestring);
    uri.data = (const char*)normalBuffURI->valuestring;
    byte* data = arena_alloc_array(arena, byte, normalBuffLen->valueint);
    
    Bytes bytes = (Bytes){data, normalBuffLen->valueint};
    if(string_contains(uri, STRING("data:"))) {
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
      resFile.data = normalBuffURI->valuestring;
      resFile.len = strlen(normalBuffURI->valuestring);

      String binFilePath = string_concatenate(arena, parentPath, resFile);
      CSTRING(binFilePath, cBinFile);
      
      FILE* file = fopen(cBinFile, "rb");
      if(!file) fprintf(stderr, "There is no binary file %s", normalBuffLen->valuestring);
      fread(data, normalBuffLen->valueint, 1, file);
      fclose(file);
    }
    hash_table_set(int, Bytes, &bufferTable, i, bytes);
  }


  for(int i = 0; i < cJSON_GetArraySize(meshes); i++) {
    const cJSON* mesh = cJSON_GetArrayItem(meshes, i);
    const cJSON* primitives = cJSON_GetObjectItemCaseSensitive(mesh, "primitives");
    for(int j = 0; j < cJSON_GetArraySize(primitives); j++) {
      const cJSON* primitive = cJSON_GetArrayItem(primitives, j);
      const cJSON* attributes = cJSON_GetObjectItemCaseSensitive(primitive, "attributes");

      const cJSON* POSITION = cJSON_GetObjectItemCaseSensitive(attributes, "POSITION");
      const cJSON* NORMAL = cJSON_GetObjectItemCaseSensitive(attributes, "NORMAL");
      const cJSON* TEXCOORD_0 = cJSON_GetObjectItemCaseSensitive(attributes, "TEXCOORD_0");
      
      Attribute position = get_attribute(arena, &bufferTable, accessors, bufferViews, buffers, POSITION);
      Attribute normal = get_attribute(arena, &bufferTable, accessors, bufferViews, buffers, NORMAL);
      Attribute texCoord = get_attribute(arena, &bufferTable, accessors, bufferViews, buffers, TEXCOORD_0);

      assert(position.count == normal.count && normal.count == texCoord.count);
      size_t vertexCount = position.count;
      float* vertexData = arena_alloc_array(arena, float, FLOATS_PER_VERTEX*vertexCount);

      size_t floatIndex = 0;
      for(size_t i = 0; i < vertexCount; i++) {
        vertexData[floatIndex++] = position.data[position.vec_type*i];
        vertexData[floatIndex++] = position.data[position.vec_type*i+1];
        vertexData[floatIndex++] = position.data[position.vec_type*i+2];

        vertexData[floatIndex++] = normal.data[normal.vec_type*i];
        vertexData[floatIndex++] = normal.data[normal.vec_type*i+1];
        vertexData[floatIndex++] = normal.data[normal.vec_type*i+2];

        vertexData[floatIndex++] = texCoord.data[texCoord.vec_type*i];
        vertexData[floatIndex++] = texCoord.data[texCoord.vec_type*i+1]; 
      }

      //index Buffer
      const cJSON* indices = cJSON_GetObjectItemCaseSensitive(primitive, "indices");
      const cJSON* indexAccessor = cJSON_GetArrayItem(accessors, indices->valueint);

      //accessor
      const cJSON* indexCount = cJSON_GetObjectItemCaseSensitive(indexAccessor, "count");
      const cJSON* indexBufferViewIndex = cJSON_GetObjectItemCaseSensitive(indexAccessor, "bufferView");
      const cJSON* indexAccOffset = cJSON_GetObjectItemCaseSensitive(indexAccessor, "byteOffset");
      const int indexAccOffsetValue = indexAccOffset ? indexAccOffset->valueint : 0;

      //buffer view
      const cJSON* indexBufferView = cJSON_GetArrayItem(bufferViews, indexBufferViewIndex->valueint);
      const cJSON* indexLen = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteLength");
      const cJSON* indexBuff = cJSON_GetObjectItemCaseSensitive(indexBufferView, "buffer");
      const cJSON* indexBuffOffset = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteOffset");
      const int indexBuffOffsetValue = indexBuffOffset ? indexBuffOffset->valueint : 0;
      
      Bytes indexBytes = hash_table_get(int, Bytes, &bufferTable, indexBuff->valueint, (Bytes){0});
      indexBytes.data += indexBuffOffsetValue + indexAccOffsetValue;
      indexBytes.len = indexLen->valueint;

      size_t indexCountValue = indexCount->valueint;

      uint32_t* indexData = arena_alloc_array(arena, uint32_t, indexCountValue);
      unsigned short* indexList = (unsigned short*)indexBytes.data;
      for(size_t i = 0; i < indexCountValue; i++) {
        indexData[i] = indexList[i];
      }

      result = gen_mesh(vertexData, indexData, vertexCount, indexCountValue);
    }
  }
  cJSON_Delete(json);
  arena_return_to_mark(&mark);
  return result;
}
*/

#endif
