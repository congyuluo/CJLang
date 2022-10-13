//
// Created by Congyu Luo on 9/29/22.
//
#include "value.h"
#include "memory.h"
#include "object.h"
#include "hashTable.h"

Table strings;

void initStrTable() {
    initTable(&strings);
}

Value allocateStringValue(char* string, int len) {
    String_Object* str_obj = (String_Object*)reallocate(NULL, 0, sizeof(String_Object));
    str_obj->length = len;
    str_obj->cString = string;
    str_obj->hash = hashString(str_obj->cString, str_obj->length);

    return MAKE_OBJ_STRING(str_obj);
}

Value makeStrValue(const char* chars, int length) {
    // Returns the interned string_value object of one with same content has been created
    // Else, create a new string object using the given char* chars
    Value newValue;

    uint32_t hash = hashString(chars, length);
    String_Object* interned = tableFindString(&strings, chars, length, hash);
    if (interned != NULL) {
        return MAKE_OBJ_STRING(interned);
    }

    newValue = allocateStringValue(chars, length);
    tableSet(&strings, newValue, MAKE_NONE);
    return newValue;
}
