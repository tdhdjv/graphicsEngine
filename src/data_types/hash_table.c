#ifndef HASH_TABLE_HEADER
#define HASH_TABLE_HEADER

#include "define.c"

#define HASH_TABLE_MAX_LOAD 0.75
#define HashTable(key_t, value_t) key_t##_##value_t##Table
#define hash_table_create(key_t, value_t, keyData, valueData, capacity) key_t##_##value_t##table_create(keyData, valueData, capacity)
#define hash_table_set(key_t, value_t, table, key, value) key_t##_##value_t##table_set(table, key, value, __FILE__, __LINE__)
#define hash_table_get(key_t, value_t, table, key, defaultValue) key_t##_##value_t##table_get(table, key, defaultValue)
#define hash_table_remove(key_t, value_t, table, key) key_t##_##value_t##table_remove(table, key, __FILE__, __LINE__)
#define hash_table_contains(key_t, value_t, table, key) key_t##_##value_t##table_contains(table, key)

#define DEFINE_HASH_TABLE(key_t, value_t, hash_func, key_is_equal, tomb, empty);\
  static const key_t tombKey = tomb;\
  static const key_t emptyKey = empty;\
\
  typedef struct {\
    key_t* keys;\
    value_t* values;\
    size_t capacity;\
    size_t length;\
  } key_t##_##value_t##Table;\
  \
  key_t##_##value_t##Table key_t##_##value_t##table_create(key_t* keyData, value_t* valueData, size_t capacity) {\
    key_t##_##value_t##Table table = {0};\
    table.keys = keyData;\
    table.values = valueData;\
    table.capacity = capacity;\
    table.length = 0;\
    for(size_t i = 0; i < capacity; i++) {\
      table.keys[i] = emptyKey;\
    }\
    \
    return table;\
  }\
  \
  void key_t##_##value_t##table_set(key_t##_##value_t##Table* table, key_t key, value_t value, const char* file, const unsigned line) {\
    size_t index = hash_func(key) % table->capacity;\
    for(size_t i = 0; i < table->capacity; i++) {\
      index++;\
      index %= table->capacity;\
      key_t indexedKey = table->keys[index];\
      if(key_is_equal(indexedKey, key)) {\
        table->values[index] = value;\
        return;\
      }\
      if(key_is_equal(indexedKey, tombKey) || key_is_equal(indexedKey, emptyKey)) {\
        if(table->length > (size_t)(table->capacity * HASH_TABLE_MAX_LOAD)) {\
          ASSERT("Hash Table Out of Size:  \n The amount of key entry is over 75%% of the capacity, increase the capacity or HASH_TABLE_MAX_LOAD to stop this!\n at file: %s, line: %u", file, line)\
        }\
        table->keys[index] = key;\
        table->values[index] = value;\
        table->length++;\
        return;\
      }\
    }\
    ASSERT("Hash Table Out of Memory: \n Hash table is out of memory, increase the capacity to stop this! \n   at file: %s, line: %u", file, line);\
  }\
\
  value_t key_t##_##value_t##table_get(key_t##_##value_t##Table* table, key_t key, value_t defaultValue) {\
    size_t index = hash_func(key) % table->capacity;\
    for(size_t i = 0; i < table->capacity; i++) {\
      index++;\
      index %= table->capacity;\
      key_t indexedKey = table->keys[index];\
      if(key_is_equal(indexedKey, key)) {\
        return table->values[index];\
      }\
      if(key_is_equal(indexedKey, emptyKey)) {\
        return defaultValue;\
      }\
    }\
    return defaultValue;\
  }\
\
  void key_t##_##value_t##table_remove(key_t##_##value_t##Table* table, key_t key, const char* file, const unsigned line) {\
    size_t index = hash_func(key) % table->capacity;\
    for(size_t i = 0; i < table->capacity; i++) {\
      index++;\
      index %= table->capacity;\
      key_t indexedKey = table->keys[index];\
      if(key_is_equal(indexedKey, key)) {\
        table->keys[index] = tombKey;\
        return;\
      }\
      if(key_is_equal(indexedKey, emptyKey)) {\
        ASSERT("Key Error: \n Hash table doesn't contain the given 'key'\n   at file: %s, line %u", file, line);\
        return;\
      }\
    }\
    ASSERT("Key Error: \n Hash table doesn't contain the given 'key'\n   at file: %s, line %u", file, line);\
  }\
\
  bool key_t##_##value_t##table_contains(key_t##_##value_t##Table* table, key_t key) {\
    size_t index = hash_func(key) % table->capacity;\
    for(size_t i = 0; i < table->capacity; i++) {\
      index++;\
      index %= table->capacity;\
      key_t indexedKey = table->keys[index];\
      if(key_is_equal(indexedKey, key)) {\
        return true;\
      }\
      if(key_is_equal(indexedKey, emptyKey)) {\
        return false;\
      }\
    }\
    return false;\
  }\


#endif
