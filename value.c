//
// Created by Congyu Luo on 9/25/22.
//
#include <math.h>
#include <stdio.h>

#include "value.h"
#include "object.h"
#include "memory.h"
#include "hashTable.h"

void initValueArray(ValueArray* array){
    array->size = 0;
    array->current_index = 0;
    array->values = NULL;
}

int valueArrayAdd(ValueArray* array, Value value) {
    if (array->current_index >= array->size) {
        int new_size = GROW_CAPACITY(array->size);
        array->values = GROW_ARRAY(Value, array->values, array->size, new_size);
        array->size = new_size;
    }

    array->values[array->current_index] = value;
    array->current_index++;

    return array->current_index - 1;
}

void resetValueArray(ValueArray* array){
    FREE_ARRAY(Value, array->values, array->size);
    initValueArray(array);
}

void printValue(Value value){
    switch (value.type) {
        case NONE_TYPE: printf("None"); return;
        case BOOL_TYPE: {
            if (value.content.bool_value) {
                printf("True");
            } else {
                printf("False");
            }
            return;
        }
        case NUMBER_TYPE: printf("%g", value.content.number_value); return;
        case OBJECT_STRING_TYPE: {
            char* string = value.content.string_object->cString;
            printf("%s", string);
            return;
        }
        default: break;
    }
    printf("Unknown Value Type");
}

char* strValueType(Value value) {
    switch (value.type) {
        case NONE_TYPE: return "NONE_TYPE";
        case BOOL_TYPE: return "BOOL_TYPE";
        case NUMBER_TYPE: return "NMBR_TYPE";
        case OBJECT_STRING_TYPE: return "OSTR_TYPE";
        default: return "_UNKNOWN_";
    }
}

void debugPrintValue(Value value) {
    switch (value.type) {
        case NONE_TYPE: printf("(NONE_T)None"); return;
        case BOOL_TYPE: {
            printf("(BOOL_T)");
            if (value.content.bool_value) {
                printf("True");
            } else {
                printf("False");
            }
            return;
        }
        case NUMBER_TYPE: printf("(NUM_T)"); printf("%g", value.content.number_value); return;
        case OBJECT_STRING_TYPE: {
            printf("(STR_T)");
            char* string = value.content.string_object->cString;
            if (value.content.string_object->length > 20){
                printf("'%.5s..'", string);
            } else {
                printf("'%s'", string);
            }
            return;
        }
        default: break;
    }
    printf("Unknown Value Type");
}

// Following three function_addrs from c tutorial from:
// https://www.geeksforgeeks.org/convert-floating-point-number-string/

// Reverses a string 'str' of length 'len'
static void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

static int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
    reverse(str, i);
    str[i] = '\0';
    return i;
}

void ftoa(float n, char* res, int afterpoint)
{
    int ipart = (int)n;
    float fpart = n - (float)ipart;
    int i = intToStr(ipart, res, 0);
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
        fpart = fpart * pow(10, afterpoint);
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
