#ifndef PT_OBJ_H
#define PT_OBJ_H

#include <cstdlib>
#include <cstdio>
#include <cstdint>

namespace Petty {
    struct PettyValue;
    
    struct PettyObject {
        uint32_t Class_ID;
        uint32_t Fields_Count;
        PettyValue* Fields;
        bool is_free;
    };

    [[noreturn]] void ThrowOutOfMemoryException();
}

#endif // PT_OBJ_H