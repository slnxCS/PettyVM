#include "PettyObject.hpp"

namespace Petty {
    [[noreturn]] void ThrowOutOfMemoryException() {
        fprintf(stderr, "OutOfMemoryException : cannot find free place to allocate new object in heap\n");
        exit(8);
    }
}