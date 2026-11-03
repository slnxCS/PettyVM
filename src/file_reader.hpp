#ifndef FILE_READER 
#define FILE_READER
#include <cstdint>

namespace Petty {
    extern uint8_t* read_file(const char* fname);
}

#endif // FILE_READER