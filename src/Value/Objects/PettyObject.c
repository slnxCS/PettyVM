#include "PettyObject.h"

inline void ThrowOutOfMemoryException() {
    fprintf(stderr, "OutOfMemoryException : cannot find free place to allocate new object in heap\n");
    exit(8);
}