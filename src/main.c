#include <sys/types.h>
#include "VM.h"
#include "file_reader.h"

#define DEBUG
#define DEBUG_FILE_NAME ("compiled.pt")

#ifdef DEBUG
int main(void) {
    byte* input = (byte*)read_file(DEBUG_FILE_NAME);
    VM_init(input, DEBUG_FILE_NAME);
    VM_Start();
    return 0;
}
#else
int main(int argc, char* argv[]) {
    return 0;
}
#endif

// source code > lexer > token[] > parser > astnode[] > semantic analyzer > compiler > byte code > virtual machine
