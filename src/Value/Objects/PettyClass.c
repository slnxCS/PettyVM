#include "PettyClass.h"
#include "../../VM/VM.h"

inline PettyClass* ClassOf(VM* vm, PettyValue val) {
    switch (val.kind)
    {
        case PT_OBJ_PTR :
            return &(vm->classes[val.as.as_obj_ptr->Class_ID]);
    
        case PT_INT32 :
            return &(vm->classes[INT32_CLASS_ID]);
        
        case PT_FLOAT32 : 
            return &(vm->classes[FLOAT32_CLASS_ID]);
        
        case PT_BOOL : 
            return &(vm->classes[BOOL_CLASS_ID]);
    }
}