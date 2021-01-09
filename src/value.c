#include <stdio.h>
#include <string.h>

#include "value.h"

Value nil_value() {
    Value v;
    v.type = VAL_TYPE_NIL;
    v.as.string = NULL;
    return v;
}

Value int_value(long x) {
    Value v;
    v.type = VAL_TYPE_INTEGER;
    v.as.integer = x;
    return v;
}

Value double_value(double x) {
    Value v;
    v.type = VAL_TYPE_DOUBLE;
    v.as.real = x;
    return v;
}

Value string_value(char *x) {
    unsigned int len = strlen(x);
    Value v;
    v.type = VAL_TYPE_STRING;
    v.as.string = malloc((len + 1) * (sizeof *v.as.string));
    memcpy(v.as.string, x, len);
    v.as.string[len] = '\0';
    return v;
}

Value bool_value(char boolean) {
    Value v;
    v.type = VAL_TYPE_BOOLEAN;
    v.as.boolean = boolean;
    return v;
}

Value add_strings(Value *s1, Value *s2) {
    unsigned int len1 = strlen(s1->as.string);
    unsigned int len2 = strlen(s2->as.string);

    Value v;
    v.type = VAL_TYPE_STRING;
    v.as.string = malloc((len1 + len2 + 1) * (sizeof *v.as.string));

    memcpy(v.as.string, s2->as.string, len2);
    memcpy(v.as.string + len2, s1->as.string, len1);
    v.as.string[len1 + len2] = '\0';

    return v;
}

void print_value(Value *v) {
    switch (v->type) {
    case VAL_TYPE_INTEGER:
        printf("%ld", v->as.integer);
        break;
    case VAL_TYPE_DOUBLE:
        printf("%lf", v->as.real);
        break;
    case VAL_TYPE_STRING:
        printf("%s", v->as.string);
        break;
    case VAL_TYPE_BOOLEAN:
        printf("%s", v->as.boolean ? "true": "false");
        break;
    case VAL_TYPE_OBJ:
        break;
    case VAL_TYPE_NIL:
        break;
    }
}
