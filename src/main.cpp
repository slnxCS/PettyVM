#include "VM/VM.hpp"
#include "file_reader.hpp"
#include <stdlib.h>
#include <time.h>
#include <iostream>

#define DEBUG

#ifdef DEBUG
#define DEBUG_FILE_NAME ("compiled.pt")

int main(void) {
    srand(time(NULL));
    uint8_t* input = (uint8_t*)Petty::read_file(DEBUG_FILE_NAME);
    Petty::VM vm(input, DEBUG_FILE_NAME);
    vm.Init();
    vm.Start();
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
