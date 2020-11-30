#ifndef _VALUE_H_
#define _VALUE_H_

#include <stdlib.h>


typedef struct value Value;
typedef struct object Object;

enum value_type {
    VAL_TYPE_NIL,
    VAL_TYPE_OBJ,
    VAL_TYPE_INTEGER,
    VAL_TYPE_DOUBLE,
    VAL_TYPE_STRING,
    VAL_TYPE_BOOLEAN
};

typedef enum object_type {
    OBJ_STRING
} object_type;

struct object {
    object_type type;
};

struct value {
    unsigned int type;
    union {
        long integer;
        double real;
        char *string;
        Object *obj;
    } as;
};

typedef struct {
    Object obj;
    unsigned int length;
    char *chars;
} string_obj;

Value nil_value();
Value int_value(long x);
Value double_value(double x);
Value string_value(char *x);

#define IS_OBJECT(value) value.type == VAL_TYPE_OBJ;

#endif /* _VALUE_H_ */
