#include <stdio.h>
#include <string.h>

#include "debugTools.h"
#include "vm.h"
#include "compiler.h"
#include "hashTable.h"
#include "object.h"
#include "makeString.h"
#include <time.h>


// This function responsible for working with sourcecode file.
// Copied from cLox code by Robert Nystrom
static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No source file given.\n");
        exit(64);
    }
    initStrTable();

    Chunk chunk;
    initChunk(&chunk);
    char* source = readFile(argv[1]);

    printf("--<TOKENIZE>--\n");
    if (!compile(source, &chunk)) {
        printf("Compile Failed");
        exit(64);
    }

    printChunk(&chunk);

    VM vm;
    initVM(&vm, &chunk);
    printf("--<RUNTIME>--\n");

    // Timing
    clock_t t;
    t = clock();

    run(&vm);

    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("Program took %f seconds to execute \n", time_taken);

    return 0;
}

static int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

//int main(int argc, const char* argv[]) {
//    // Timing
//    clock_t t;
//    t = clock();
//
//    printf("%d\n", fib(30));
//
//    t = clock() - t;
//    double time_taken = ((double)t)/CLOCKS_PER_SEC;
//    printf("Program took %f seconds to execute \n", time_taken);
//
//}
