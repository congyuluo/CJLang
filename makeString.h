//
// Created by Congyu Luo on 9/29/22.
//

#ifndef CJLANG_MAKESTRING_H
#define CJLANG_MAKESTRING_H

void initStrTable();

Value allocateStringValue(char* string, int len);
Value makeStrValue(char* chars, int length);

#endif //CJLANG_MAKESTRING_H
