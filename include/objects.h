#ifndef _VALUE_H_
#define _VALUE_H_

typedef enum {
    VAL_TYPE_NIL,
    VAL_TYPE_INTEGER,
    VAL_TYPE_DOUBLE,
    VAL_TYPE_STRING,
    VAL_TYPE_BOOLEAN
} value_type;

typedef struct {
    unsigned int type;
    union {
        long l;
        double d;
        char *str;
    } as;
} value;

typedef struct {

} string_obj;

#endif /* _VALUE_H_ */
