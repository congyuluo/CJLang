//
// Created by Congyu Luo on 9/25/22.
//

#ifndef CJLANG_DEBUGTOOLS_H
#define CJLANG_DEBUGTOOLS_H

#include "chunk.h"
#include "vm.h"
#include "token.h"

void printChunk(Chunk* chunk);
void printOp(uint8_t opCode);
void printStack(VM* vm);
void printToken(Token token);

#endif //CJLANG_DEBUGTOOLS_H
