#include "file_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char* read_file(char* fname) {
    if (!fname) return NULL;

    FILE* f = fopen(fname, "rb");

    if (!f) 
    {
        fprintf(stderr,"File '%s' does not exist\n", fname);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    uint32_t file_lenght = ftell(f);
    rewind(f);
    char* buffer = malloc(file_lenght);
    if (!buffer) {
        fprintf(stderr,"Memory allocation error\n");
        exit(1);
    }

    fread(buffer, 1, file_lenght, f);
    fclose(f);
    return buffer;
}