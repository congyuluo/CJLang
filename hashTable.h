//
// Created by Congyu Luo on 9/28/22.
//

#ifndef CJLANG_HASHTABLE_H
#define CJLANG_HASHTABLE_H

#include "value.h"

typedef struct {
    String_Object* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableGet(Table* table, Value key_value, Value* value);
bool tableSet(Table* table, Value key_value, Value value);
bool tableDelete(Table* table, Value key_value);
String_Object* tableFindString(Table* table, const char* chars, int length, uint32_t hash);

#endif //CJLANG_HASHTABLE_H
