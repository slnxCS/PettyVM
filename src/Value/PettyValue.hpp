#ifndef PT_VAL_H
#define PT_VAL_H

#include <cstdint>

namespace Petty {
    struct PettyObject;
    struct PettyValue {
        enum class Kind : uint8_t {
            PT_INT32,
            PT_FLOAT32,
            PT_BOOL,
            PT_OBJ_PTR,
        };

        Kind kind;
        union {
            bool as_bool;
            int32_t as_int;
            _Float32 as_float;
            PettyObject* as_obj_ptr;
        } as;

        PettyValue() {}
        PettyValue(int32_t as_int) { kind = Kind::PT_INT32; as.as_int = as_int; }
        PettyValue(bool as_bool) { kind = Kind::PT_BOOL; as.as_bool = as_bool; }
        PettyValue(_Float32 as_float) { kind = Kind::PT_FLOAT32; as.as_float = as_float; }
        PettyValue(PettyObject* as_obj_ptr) { kind = Kind::PT_OBJ_PTR; as.as_obj_ptr = as_obj_ptr; }
    };
}

#endif // PT_VAL_H