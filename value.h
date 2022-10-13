//
// Created by Congyu Luo on 9/25/22.
//

#ifndef CJLANG_VALUE_H
#define CJLANG_VALUE_H

#include "imports.h"

//typedef struct Object Object;
typedef struct String_Object String_Object;

typedef enum {
    NONE_TYPE,
    NUMBER_TYPE,
    BOOL_TYPE,
    OBJECT_STRING_TYPE,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool bool_value;
        double number_value;
        String_Object* string_object;
    } content;
} Value;

typedef struct {
    int size;
    int current_index;
    Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
int valueArrayAdd(ValueArray* array, Value value);
void resetValueArray(ValueArray* array);

#define MAKE_NONE ((Value){NONE_TYPE, {.bool_value = false}})
#define MAKE_NUMBER(value) ((Value){NUMBER_TYPE, {.number_value = value}})
#define MAKE_BOOL(value) ((Value){BOOL_TYPE, {.bool_value = value}})

#define MAKE_OBJ_STRING(obj_ptr) ((Value){OBJECT_STRING_TYPE, {.string_object = obj_ptr}})

void printValue(Value value);

char* strValueType(Value value);

void debugPrintValue(Value value);

// Using function_addrs from tutorial to convert double to char*
// https://www.geeksforgeeks.org/convert-floating-point-number-string/
void ftoa(float n, char* res, int afterpoint);

#endif //CJLANG_VALUE_H
