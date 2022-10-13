//
// Created by Congyu Luo on 9/28/22.
//

#ifndef CJLANG_COMPILER_H
#define CJLANG_COMPILER_H

#include "vm.h"

bool compile(const char* source, Chunk* chunk);

#endif //CJLANG_COMPILER_H
