//
// Created by Congyu Luo on 9/25/22.
//

#ifndef CJLANG_VM_H
#define CJLANG_VM_H

#include "chunk.h"
#include "hashTable.h"

#define STACK_LIMIT 256

typedef struct{
    Value key;
    int index;
    int scope;
} Local;

typedef struct {
    Chunk* chunk;
    uint8_t* instruction_pointer;
    // VM stack
    Value stack[STACK_LIMIT];
    int stack_index;
    Value* stackTop;
    // Function callstack
    Value ra_stack[STACK_LIMIT];
    int ra_stack_index;
    Value* ra_stackTop;
    // Local array
    Local locals[STACK_LIMIT];
    int local_index;

    Value returnValue;

    bool hasError;
    int scope;
    Table globals;
} VM;

typedef enum {
    RUNTIME_FAILURE,
    COMPILE_FAILURE,
    RUNTIME_SUCCESS,
} OperationResult;

void initVM(VM* vm, Chunk* chunk);
OperationResult run(VM* vm);

#endif //CJLANG_VM_H
