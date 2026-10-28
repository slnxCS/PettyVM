#ifndef ARRAY_TOOLS_H
#define ARRAY_TOOLS_H
#include "PettyValue.h"

#define ARRAY_CLASS_ID (7)

#define IS_OBJ_ARRAY(obj) (obj.kind == PT_OBJ_PTR && obj->Class_ID == 7)
#define ARRAY_GET(obj, index) (obj->Fields[index])
#define ARRAY_SET(obj, index, value) (obj->Fields[index] = value)

#endif // ARRAY_TOOLS_H