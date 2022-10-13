//
// Created by Congyu Luo on 9/25/22.
//

#ifndef CJLANG_CHUNK_H
#define CJLANG_CHUNK_H

#include "imports.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_TYPE,
    OP_GET_LEN,
    OP_GET_TIME,
    OP_GET_VAR,
    OP_SET_GLOBAL,
    OP_SET_VAR,
    OP_ASSIGN_LOCAL,
    OP_UP_SCOPE,
    OP_DOWN_SCOPE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EXPONENT,
    OP_MOD,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_PRINTLN,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_FALSE_DISCARD,
    OP_JUMP_IF_TRUE,
    OP_LOOP,
    OP_CALL,
    OP_RA_PUSH,
    OP_RV_POP,
    OP_RETURN,
} OpCode;

typedef struct {
    int size;
    int current_index;
    uint8_t* bytecode_array;
    ValueArray constant_array;
} Chunk;

void initChunk(Chunk* chunk);
void resetChunk(Chunk* chunk);
void chunkAdd(Chunk* chunk, uint8_t code);
void chunkAddConstant(Chunk* chunk, Value constant);

#endif //CJLANG_CHUNK_H
