#include <glad/glad.h>
#include <cJSON.h>

#include <stdio.h>

#include "../scene/scene_defines.c"

#include "../data_types/io.c"
#include "../data_types/arena.c"
#include "../data_types/hashtable.c"
#include "../scene/mesh.c"

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

DEFINE_HASH_TABLE(int, Bytes, mesh_hash, int_is_equal)

typedef struct {
  byte* data;
  size_t bytesPerElement;
  size_t stride;
  size_t count;
} GLTFAttribute;

GLTFAttribute load_attribute_from_gltf(
    Arena* arena, HashTable(int, Bytes)* bufferTable, const cJSON* accessors, const cJSON* bufferViews, const cJSON* buffers, 
    const cJSON* attribute, GLenum expectedComponentType, uint8_t expectedVecType) {
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
  Bytes buffer = hash_table_get(int, Bytes, bufferTable, bufferIndex->valueint, (Bytes){0});
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
  if(string_equals(typeStr, STRING("SCALAR"))) vecType = 1;
  else if(string_equals(typeStr, STRING("VEC2"))) vecType = 2;
  else if(string_equals(typeStr, STRING("VEC3"))) vecType = 3;
  else if(string_equals(typeStr, STRING("VEC4"))) vecType = 4;

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

Geometry load_geometry_from_gltf(Arena* arena, HashTable(int, Bytes)* bufferTable, const cJSON* accessors, const cJSON* bufferViews, const cJSON* buffers, const cJSON* primitive) {
  const cJSON* attributes = cJSON_GetObjectItemCaseSensitive(primitive, "attributes");

  const cJSON* POSITION = cJSON_GetObjectItemCaseSensitive(attributes, "POSITION");
  const cJSON* NORMAL = cJSON_GetObjectItemCaseSensitive(attributes, "NORMAL");
  const cJSON* TEXCOORD_0 = cJSON_GetObjectItemCaseSensitive(attributes, "TEXCOORD_0");

  
  GLTFAttribute position = load_attribute_from_gltf(arena, bufferTable, accessors, bufferViews, buffers, POSITION, GL_FLOAT, 3);
  GLTFAttribute normal = load_attribute_from_gltf(arena, bufferTable, accessors, bufferViews, buffers, NORMAL, GL_FLOAT, 3);
  GLTFAttribute texCoord = load_attribute_from_gltf(arena, bufferTable, accessors, bufferViews, buffers, TEXCOORD_0, GL_FLOAT, 3);

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
  const int indexAccOffsetValue = indexAccOffset ? indexAccOffset->valueint : 0;

  //buffer view
  const cJSON* indexBufferView = cJSON_GetArrayItem(bufferViews, indexBufferViewIndex->valueint);
  const cJSON* indexLen = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteLength");
  const cJSON* indexBuff = cJSON_GetObjectItemCaseSensitive(indexBufferView, "buffer");
  const cJSON* indexBuffOffset = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteOffset");
  const int indexBuffOffsetValue = indexBuffOffset ? indexBuffOffset->valueint : 0;
  
  Bytes indexBytes = hash_table_get(int, Bytes, bufferTable, indexBuff->valueint, (Bytes){0});
  indexBytes.data += indexBuffOffsetValue + indexAccOffsetValue;
  indexBytes.len = indexLen->valueint;

  size_t indexCountValue = indexCount->valueint;

  uint32_t* indexData = arena_alloc_array(arena, uint32_t, indexCountValue);
  unsigned short* indexList = (unsigned short*)indexBytes.data;
  for(size_t i = 0; i < indexCountValue; i++) {
    indexData[i] = indexList[i];
  }
  
  Attribute* attributeData = arena_alloc_array(arena, Attribute, 3);
  Array(Attribute) attributeList = create_array(Attribute, attributeData, 3);
  Attribute positonAttrib = (Attribute){GL_FLOAT, 3};
  Attribute normalAttrib = (Attribute){GL_FLOAT, 3};
  Attribute texCoordAttrib = (Attribute){GL_FLOAT, 2};
  array_set(Attribute, &attributeList, 0, positonAttrib);
  array_set(Attribute, &attributeList, 1, normalAttrib);
  array_set(Attribute, &attributeList, 2, texCoordAttrib);

  return generate_geometry(vertexData, vertexCount, &attributeList, indexData, indexCountValue);
}

Model load_model_from_gltf(Arena* arena) {
  return (Model){0};
}

/*

Scene load_scene_from_gltf(Arena* arena, String filePath) {
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

  //Adding Models
  for(int i = 0; i < cJSON_GetArraySize(meshes); i++) {
    const cJSON* mesh = cJSON_GetArrayItem(meshes, i);
    const cJSON* primitives = cJSON_GetObjectItemCaseSensitive(mesh, "primitives");
    Model model = (Model){0};
    //Adding Meshes
    for(int j = 0; j < cJSON_GetArraySize(primitives); j++) {
      Mesh mesh = (Mesh){0};

      const cJSON* primitive = cJSON_GetArrayItem(primitives, j);
      const cJSON* attributes = cJSON_GetObjectItemCaseSensitive(primitive, "attributes");

      const cJSON* POSITION = cJSON_GetObjectItemCaseSensitive(attributes, "POSITION");
      const cJSON* NORMAL = cJSON_GetObjectItemCaseSensitive(attributes, "NORMAL");
      const cJSON* TEXCOORD_0 = cJSON_GetObjectItemCaseSensitive(attributes, "TEXCOORD_0");
      
      GLTFAttribute position = get_attribute(arena, &bufferTable, accessors, bufferViews, buffers, POSITION, GL_FLOAT);
      GLTFAttribute normal = get_attribute(arena, &bufferTable, accessors, bufferViews, buffers, NORMAL, GL_FLOAT);
      GLTFAttribute texCoord = get_attribute(arena, &bufferTable, accessors, bufferViews, buffers, TEXCOORD_0, GL_FLOAT);

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
    }
  }
  cJSON_Delete(json);
  arena_return_to_mark(&mark);
}

*/

#include "../data_types/array.c"

Geometry extract_geometry_from_gltf(Arena* arena, String filePath) {
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

  //Adding Models
  for(int i = 0; i < cJSON_GetArraySize(meshes); i++) {
    const cJSON* mesh = cJSON_GetArrayItem(meshes, i);
    const cJSON* primitives = cJSON_GetObjectItemCaseSensitive(mesh, "primitives");
    //Adding Meshes
    for(int j = 0; j < cJSON_GetArraySize(primitives); j++) {
      const cJSON* prim = cJSON_GetArrayItem(primitives, i);
      return load_geometry_from_gltf(arena, &bufferTable, accessors, bufferViews, buffers, prim);
    }
  }
  return (Geometry){0};
}

