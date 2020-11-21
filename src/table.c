#include <stdlib.h>
#include <string.h>

#include "table.h"

#define FNV1_32_INIT 2166136261u
#define FNV1_32_PRIME 16777619u

/* FNV-1a */
static uint32_t hash_bytes(const void* key, unsigned int length) {
  uint32_t hash = FNV1_32_INIT;

  for (int i = 0; i < length; i++) {
    hash ^= ((uint8_t*) key)[i];
    hash *= FNV1_32_PRIME;
  }

  return hash;
}

#undef FNV1_32_INIT
#undef FNV1_32_PRIME


uint32_t hash_long(const long num) {
    return hash_bytes(&num, sizeof(long));
}

uint32_t hash_string(const char *str) {
    return hash_bytes(str, strlen(str));
}

// key make_key(value *val) {
//     uint32_t hash;

//     switch(val->type) {
//     case VAL_TYPE_STRING:
//         hash = hash_string(val->str);
//         break;
//     }

//     return 
// }

// value *find_table_value(table *table, key *k) {
//     uint32_t index = k->hash % table->capacity;

//     for (;;) {
//         entry *e = &table->entries[index];

//         if (e->key == k || e->key == NULL) {
//             return e;
//         }

//         index = (index + 1) % table->capacity;
//     }
// }