#ifndef HASH_TABLE_HEADER
#define HASH_TABLE_HEADER

#include <stdint.h>
#include <stddef.h>
#include <malloc.h>
#include <stdbool.h>
#include <memory.h>

#define HashTable(key_t, value_t) key_t##_##value_t##_Table
#define KeyValue(key_t, value_t) key_t##_##value_t##_Pair
#define create_hash_table(key_t, value_t, data, capacity) create_##key_t##_##value_t##_table(data, capacity)
#define hash_table_set(key_t, value_t, table, key, value) key_t##_##value_t##_table_set(table, key, value)
#define hash_table_get(key_t, value_t, table, key, defaultValue) key_t##_##value_t##_table_get(table, key, defaultValue)
#define hash_table_contains(key_t, value_t, table, key) key_t##_##value_t##_table_contains(table, key)

#define DEFINE_HASH_TABLE(key_t, value_t, hashFunc, isEqual);\
  typedef struct {\
    bool taken;\
    key_t key;\
    value_t value;\
  } key_t##_##value_t##_Pair;\
\
  typedef struct {\
    key_t##_##value_t##_Pair* table;\
    size_t capacity;\
  } key_t##_##value_t##_Table;\
\
  key_t##_##value_t##_Pair* key_t##_##_value_t##_##_table_look_up(key_t##_##value_t##_Table* table, key_t key) {\
    uint64_t hashValue = hashFunc(key);\
    for(size_t index = (hashValue%table->capacity); index < table->capacity; index++) {\
      if(!table->table[index].taken || isEqual(table->table[index].key, key)) return &table->table[index];\
    }\
    assert(false);\
  }\
\
  void key_t##_##value_t##_table_set(key_t##_##value_t##_Table* table, key_t key, value_t value) {\
    key_t##_##value_t##_Pair* pair = key_t##_##_value_t##_##_table_look_up(table, key);\
    pair->taken = true;\
    pair->key = key;\
    pair->value = value;\
  }\
\
  value_t key_t##_##value_t##_table_get(key_t##_##value_t##_Table* table, key_t key, value_t default_value) {\
    key_t##_##value_t##_Pair* pair = key_t##_##_value_t##_##_table_look_up(table, key);\
    if(pair->taken) return pair->value;\
    return default_value;\
  }\
\
  bool key_t##_##value_t##_table_contains(key_t##_##value_t##_Table* table, key_t key) {\
    key_t##_##value_t##_Pair* pair = key_t##_##_value_t##_##_table_look_up(table, key);\
    return pair->taken;\
  }\
\
  key_t##_##value_t##_Table create_##key_t##_##value_t##_table(key_t##_##value_t##_Pair* data, size_t capacity) {\
    for(size_t i = 0; i < capacity; i++) data[i].taken = false;\
    key_t##_##value_t##_Table table = (key_t##_##value_t##_Table){.table=data, .capacity=capacity};\
    return table;\
  }\
\
  void free_##key_t##_##value_t##_table(key_t##_##value_t##_Table* table) {\
    free(table->table);\
    table->capacity = 0;\
    table->table = NULL;\
  }

#endif
