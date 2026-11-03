#include "PettyObject.hpp"
#include "PettyClass.hpp"
#include "VM/VM.hpp"
#include "Value/PettyValue.hpp"

namespace Petty {
    const PettyClass* ClassOf(VM* vm, PettyValue val) {
        switch (val.kind)
        {
            case PettyValue::Kind::PT_OBJ_PTR :
                return &(vm->Classes()[val.as.as_obj_ptr->Class_ID]);
        
            case PettyValue::Kind::PT_INT32 :
                return &(vm->Classes()[INT32_CLASS_ID]);

            case PettyValue::Kind::PT_FLOAT32 : 
                return &(vm->Classes()[FLOAT32_CLASS_ID]);

            case PettyValue::Kind::PT_BOOL : 
                return &(vm->Classes()[BOOL_CLASS_ID]);
        }
        return nullptr;
    }
}