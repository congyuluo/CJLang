//
// Created by Congyu Luo on 9/25/22.
//

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->size = 0;
    chunk->current_index = 0;
    chunk->bytecode_array = NULL;
    initValueArray(&chunk->constant_array);
}

void resetChunk(Chunk* chunk) {
    // Reset constant array
    resetValueArray(&chunk->constant_array);
    // Reset bytecodes
    FREE_ARRAY(uint8_t, chunk->bytecode_array, chunk->size);
    initChunk(chunk);
}

void chunkAdd(Chunk* chunk, uint8_t code) {
    // Expand if needed
    if (chunk->current_index >= chunk->size) {
        int new_size = GROW_CAPACITY(chunk->size);
        chunk->bytecode_array = GROW_ARRAY(uint8_t, chunk->bytecode_array, chunk->size, new_size);
        chunk->size = new_size;
    }

    chunk->bytecode_array[chunk->current_index] = code;
    chunk->current_index++;
}

void chunkAddConstant(Chunk* chunk, Value constant) {
    // Add constant to constant array
    int added_index = valueArrayAdd(&chunk->constant_array, constant);
    chunkAdd(chunk, added_index);
}


