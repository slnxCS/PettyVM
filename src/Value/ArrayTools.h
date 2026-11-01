#ifndef ARRAY_TOOLS_H
#define ARRAY_TOOLS_H
#include "PettyValue.h"
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_CLASS_ID (7)

#define IS_OBJ_ARRAY(obj) (obj->Class_ID == ARRAY_CLASS_ID)
#define ARRAY_GET(obj, index) (obj->Fields[index])
#define ARRAY_SET(obj, index, value) (obj->Fields[index] = value)
#define ARRAY_LEN(obj) (obj->Fields_Count)

static inline void ThrowOutOfRangeException(PettyObject* array, int32_t index) {
    fprintf(stderr, "OutOfRangeException : Index(%d) was outside the bounds of the array with length %d", index, ARRAY_LEN(array));
    exit(15);
}

static inline void Array_ThrowIfOutOfRange(PettyObject* obj, int32_t index) {
    if (!IS_OBJ_ARRAY(obj) || index < 0 || index >= ARRAY_LEN(obj)) {
        ThrowOutOfRangeException(obj, index);
    }
}

#endif // ARRAY_TOOLS_H