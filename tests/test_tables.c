#include <string.h>

#include "table.h"
#include "value.h"

void test_add_entry() {
    HashTable *table = init_table();
    char *key1 = "hi";
    add_entry(table, key1, string_value("hello"));

    char *keys[3];
    char *vals[3];
    for (unsigned int i = 0; i < 3; i++) {
        keys[i] = malloc(sizeof(char) * 5);
        vals[i] = malloc(sizeof(char) * 5);
        snprintf(keys[i], 5, "key%d", i);
        snprintf(vals[i], 5, "val%d", i);

        add_entry(table, keys[i], string_value(vals[i]));
    }

    for (unsigned int i = 0; i < 3; i++) {
        Entry *e = get_entry(table, keys[i]);

        if (e == NULL) {
            printf("Failed\n");
        }

        if (strcmp(e->value.as.string, vals[i]) != 0) {
            printf("Failed\n");
        }
    }

    free_table(table);
    for (unsigned int i = 0; i < 3; i++) {
        free(keys[i]);
        free(vals[i]);
    }
}

int main() {
    test_add_entry();

    return 0;
}
