#ifndef MINIC_TYPE_H
#define MINIC_TYPE_H

typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_ERROR,
    TYPE_UNKNOWN
} Type;

const char *type_to_string(Type type);
int types_are_compatible(Type expected, Type actual);

#endif
