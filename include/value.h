/** @file value.h */
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
        unsigned char boolean;
        char *string;
        Object *obj;
    } as;
};

typedef struct {
    Object obj;
    unsigned int length;
    char *chars;
} string_obj;

/**
 * Creates a Value with type nil.
 *
 * @return A Value struct
 */
Value nil_value();

/**
 * Creates a Value with type integer.
 *
 * @param x An integer value.
 * @return A Value struct.
 */
Value int_value(long x);

/**
 * Creates a Value with type double.
 *
 * @param x A double value.
 * @return A Value struct
 */
Value double_value(double x);

/**
 * Creates a Value with type string.
 *
 * @param x A char array.
 * @return A Value struct.
 */
Value string_value(char *x);

/**
 * Creates a Value from two strings.
 *
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A Value struct.
 */
Value add_strings(Value *s1, Value *s2);

/**
 * Prints a value.
 *
 * @param v A pointer to the value to print.
 */
void print_value(Value *v);

#define IS_OBJECT(value) value.type == VAL_TYPE_OBJ;

#endif /* _VALUE_H_ */
