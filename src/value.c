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
    Value v;
    v.type = VAL_TYPE_STRING;
    v.as.string = x;
    return v;
}
