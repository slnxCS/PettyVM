#ifndef ARRAY_TOOLS_H
#define ARRAY_TOOLS_H
#include "PettyValue.hpp"
#include "Objects/PettyObject.hpp"
#include <cstdio>
#include <cstdlib>

#define ARRAY_CLASS_ID (7)

#define IS_OBJ_ARRAY(obj) (obj->Class_ID == ARRAY_CLASS_ID)

namespace Petty {
    static inline PettyValue PTArrayGet(PettyObject* ar, uint32_t index) {
        return ar->Fields[index];
    }

    static inline void PTArraySet(PettyObject* ar, uint32_t index, PettyValue val) {
        ar->Fields[index] = val;
    }

    static inline uint32_t PTArrayLen(PettyObject* ar) {
        return ar->Fields_Count;
    }

    static inline void ThrowOutOfRangeException(PettyObject* array, int32_t index) {
        fprintf(stderr, "OutOfRangeException : Index(%d) was outside the bounds of the array with length %d", index, PTArrayLen(array));
        exit(15);
    }

    static inline void Array_ThrowIfOutOfRange(PettyObject* obj, uint32_t index) {
        if (!IS_OBJ_ARRAY(obj)) return;
        if (index >= PTArrayLen(obj)) {
            ThrowOutOfRangeException(obj, index);
        }
    }
}

#endif // ARRAY_TOOLS_H