#include <cJSON.h>
#include <stdio.h>
#include "src/data_types/io.c"
#include "src/data_types/arena.c"
#include "src/data_types/hashtable.c"

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

uint64_t hash(int value) {
  return value;
}

bool is_equal(int a, int b) { return a == b; }

DEFINE_HASH_TABLE(int, Bytes, hash, is_equal)

void gen_gltf(Arena* arena, const char* filePath) {
  String source = read_file(arena, "Avocado.gltf");
  CSTRING(source, cSource)
  cJSON *json = cJSON_Parse(cSource);
  assert(json);

  const cJSON* meshes = cJSON_GetObjectItemCaseSensitive(json, "meshes");
  const cJSON* accessors = cJSON_GetObjectItemCaseSensitive(json, "accessors");
  const cJSON* bufferViews = cJSON_GetObjectItemCaseSensitive(json, "bufferViews");
  const cJSON* buffers = cJSON_GetObjectItemCaseSensitive(json, "buffers");
  size_t bufferCount = cJSON_GetArraySize(buffers);
  KeyValue(int, Bytes)* data = arena_alloc_array(arena, KeyValue(int, Bytes), bufferCount);
  HashTable(int, Bytes) buffDataTable = create_hash_table(int, Bytes, data, sizeof(KeyValue(int, Bytes))*bufferCount);

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
      FILE* file = fopen(normalBuffURI->valuestring, "rb");
      fread(data, sizeof(data), 1, file);
      fclose(file);
    }
    
    hash_table_set(int, Bytes, &buffDataTable, i, bytes);
  }

  for(int i = 0; i < cJSON_GetArraySize(meshes); i++) {
    const cJSON* mesh = cJSON_GetArrayItem(meshes, i);
    const cJSON* primitives = cJSON_GetObjectItemCaseSensitive(mesh, "primitives");
    for(int j = 0; j < cJSON_GetArraySize(primitives); j++) {
      const cJSON* primitive = cJSON_GetArrayItem(primitives, j);
      const cJSON* attributes = cJSON_GetObjectItemCaseSensitive(primitive, "attributes");

      const cJSON* NORMAL = cJSON_GetObjectItemCaseSensitive(attributes, "NORMAL");
      const cJSON* POSITION = cJSON_GetObjectItemCaseSensitive(attributes, "POSITION");
      const cJSON* TEXCOORD_0 = cJSON_GetObjectItemCaseSensitive(attributes, "TEXCOORD_0");
      
      //accesors
      const cJSON* normalAttribute = cJSON_GetArrayItem(accessors, NORMAL->valueint);
      const cJSON* normalBuffViewIndex = cJSON_GetObjectItemCaseSensitive(normalAttribute, "bufferView");
      const cJSON* normalComponentType = cJSON_GetObjectItemCaseSensitive(normalAttribute, "componentType");
     // const cJSON* normalCount = cJSON_GetObjectItemCaseSensitive(normalAttribute, "count");
      //const cJSON* normalType = cJSON_GetObjectItemCaseSensitive(normalAttribute, "type");

      assert(normalComponentType->valueint == 5126);


      const cJSON* positionAttribute = cJSON_GetArrayItem(accessors, POSITION->valueint);
      const cJSON* positionBuffViewIndex = cJSON_GetObjectItemCaseSensitive(positionAttribute, "bufferView");
      const cJSON* positionComponentType = cJSON_GetObjectItemCaseSensitive(positionAttribute, "componentType");
      //const cJSON* positionCount = cJSON_GetObjectItemCaseSensitive(positionAttribute, "count");
      //const cJSON* positionType = cJSON_GetObjectItemCaseSensitive(positionAttribute, "type");

      assert(positionComponentType->valueint == 5126);

      const cJSON* texCoordAttribute = cJSON_GetArrayItem(accessors, TEXCOORD_0->valueint);
      const cJSON* texCoordBuffViewIndex = cJSON_GetObjectItemCaseSensitive(texCoordAttribute, "bufferView");
      const cJSON* texCoordComponentType = cJSON_GetObjectItemCaseSensitive(texCoordAttribute, "componentType");
      //const cJSON* texCoordCount = cJSON_GetObjectItemCaseSensitive(texCoordAttribute, "count");
      //const cJSON* texCoordType = cJSON_GetObjectItemCaseSensitive(texCoordAttribute, "type");

      assert(texCoordComponentType->valueint == 5126);
      
      //buffer view
      const cJSON* normalBuffView = cJSON_GetArrayItem(bufferViews, normalBuffViewIndex->valueint);
      const cJSON* positionBuffView = cJSON_GetArrayItem(bufferViews, positionBuffViewIndex->valueint);
      const cJSON* texCoordBuffView = cJSON_GetArrayItem(bufferViews, texCoordBuffViewIndex->valueint);

      const cJSON* normalBuff = cJSON_GetObjectItemCaseSensitive(normalBuffView, "buffer");
      const cJSON* normalLen = cJSON_GetObjectItemCaseSensitive(normalBuffView, "byteLength");
      const cJSON* normalOffset = cJSON_GetObjectItemCaseSensitive(normalBuffView, "byteOffset");
      //const cJSON* normalStride = cJSON_GetObjectItemCaseSensitive(normalBuffView, "byteStride");

      const cJSON* positionBuff = cJSON_GetObjectItemCaseSensitive(positionBuffView, "buffer");
      const cJSON* positionLen = cJSON_GetObjectItemCaseSensitive(positionBuffView, "byteLength");
      const cJSON* positionOffset = cJSON_GetObjectItemCaseSensitive(positionBuffView, "byteOffset");
      //const cJSON* positionStride = cJSON_GetObjectItemCaseSensitive(normalBuffView, "byteStride");

      const cJSON* texCoordBuff = cJSON_GetObjectItemCaseSensitive(texCoordBuffView, "buffer");
      const cJSON* texCoordLen = cJSON_GetObjectItemCaseSensitive(texCoordBuffView, "byteLength");
      const cJSON* texCoordOffset = cJSON_GetObjectItemCaseSensitive(texCoordBuffView, "byteOffset");
      //const cJSON* texCoordStride = cJSON_GetObjectItemCaseSensitive(normalBuffView, "byteStride");

      const int normalOffsetValue = normalOffset ? normalOffset->valueint : 0, positionOffsetValue = positionOffset ? positionOffset->valueint : 0, texCoordOffsetValue = texCoordOffset ? texCoordOffset->valueint:0; 

      assert(positionBuff->valueint == normalBuff->valueint && normalBuff->valueint == texCoordBuff->valueint);

      Bytes vertexBytes = hash_table_get(int, Bytes, &buffDataTable, normalBuff->valueint, (Bytes){0});
      size_t positionSize = positionLen->valueint + positionOffsetValue, normalSize = normalLen->valueint + normalOffsetValue, texCoordSize = texCoordLen->valueint + texCoordOffsetValue;
      size_t vertexSize = positionSize > normalSize ? (texCoordSize > positionSize ? texCoordSize : positionSize) : (texCoordSize > normalSize ? texCoordSize : normalSize);
      //index Buffer
      const cJSON* indices = cJSON_GetObjectItemCaseSensitive(primitive, "indices");
      const cJSON* indexAttribute = cJSON_GetArrayItem(accessors, indices->valueint);

      const cJSON* indexCount = cJSON_GetObjectItemCaseSensitive(indexAttribute, "count");
      const cJSON* indexBufferViewIndex = cJSON_GetObjectItemCaseSensitive(indexAttribute, "bufferView");

      const cJSON* indexBufferView = cJSON_GetArrayItem(bufferViews, indexBufferViewIndex->valueint);
      const cJSON* indexLen = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteLength");
      const cJSON* indexOffset = cJSON_GetObjectItemCaseSensitive(indexBufferView, "byteOffset");
      
      const cJSON* indexBuff = cJSON_GetObjectItemCaseSensitive(indexBufferView, "buffer");
      
      const int indexOffsetValue = indexOffset ? indexOffset->valueint : 0;
      
      Bytes indexBytes = hash_table_get(int, Bytes, &buffDataTable, indexBuff->valueint, (Bytes){0});
      indexBytes.data += indexOffsetValue;
      indexBytes.len = indexLen->valueint;

      printf("%d, %zu", indexCount->valueint, vertexSize);
      printf("%zu %p", vertexSize, vertexBytes.data);
      printf("%zu %p", indexBytes.len, indexBytes.data);

      printf("%p %p %p", (void*)((size_t)positionOffsetValue), (void*)((size_t)normalOffsetValue), (void*)((size_t)texCoordOffsetValue));
      cJSON_Delete(json);
      return;
    }
  }
  cJSON_Delete(json);
  return;
}

int main(void) {
  size_t size = 1<<24;
  char* data = malloc(size);
  Arena arena = create_arena(data, size);
  gen_gltf(&arena, "res/cube.gltf");
  free_arena(&arena);
  return 0;
}

