#include "types.h"
#include "VM/VM.h"
#include "file_reader.h"
#include <stdlib.h>
#include <time.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_FILE_NAME ("compiled.pt")

int main(void) {
    srand(time(NULL));
    VM vm;
    byte* input = (byte*)read_file(DEBUG_FILE_NAME);
    VM_init(&vm, input, DEBUG_FILE_NAME);
    VM_Start(&vm);
    VM_Free(&vm);
    return 0;
}
#else
#include <stdio.h>
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Please, provide soruce file\n");
        return 6;
    }
    else if (argc > 2) {
        fprintf(stderr, "To many arguments provided!\n");
        return 6;
    }
    VM vm;
    srand(time(NULL));
    char* fname = argv[1];
    byte* input = (byte*)read_file(fname);
    VM_init(&vm, input, fname);
    VM_Start(&vm);
    VM_Free(&vm);
    return 0;
    return 0;
}
#endif

// source code > lexer > token[] > parser > astnode[] > semantic analyzer > compiler > byte code > virtual machine
