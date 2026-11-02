#include "semantic/type.h"

const char*
type_to_string(Type type)
{
    switch (type) {
    case TYPE_INT:
        return "int";
    case TYPE_BOOL:
        return "bool";
    case TYPE_ERROR:
        return "error";
    case TYPE_UNKNOWN:
        return "unknown";
    default:
        return "?";
    }
}

int
types_are_compatible(Type expected, Type actual)
{
    return expected == actual
           && expected != TYPE_ERROR
           && expected != TYPE_UNKNOWN;
}
