//
// Created by Congyu Luo on 9/25/22.
//

#ifndef CJLANG_OBJECT_H
#define CJLANG_OBJECT_H

#include "value.h"

struct String_Object {
    int length;
    char* cString;
    uint32_t hash;
};

char* copyString(const char* chars, int length);
uint32_t hashString(const char* key, int length);

#endif //CJLANG_OBJECT_H
