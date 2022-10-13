//
// Created by Congyu Luo on 9/26/22.
//

#ifndef CJLANG_TOKEN_H
#define CJLANG_TOKEN_H

// This enum responsible for indicating token types.
// Copied & modified from cLox code by Robert Nystrom
typedef enum {
    // Single-character tokens.
    LEFT_PAREN_T, RIGHT_PAREN_T,
    LEFT_BRACE_T, RIGHT_BRACE_T,
    COMMA_T, DOT_T, MINUS_T, MINUS_EQUAL_T, PLUS_T, PLUS_EQUAL_T,
    SEMICOLON_T, SLASH_T, SLASH_EQUAL_T, STAR_T, STAR_EQUAL_T, CARET_T, CARET_EQUAL_T,
    MOD_T, MOD_EQUAL_T,
    // One or two character tokens.
    BANG_T, BANG_EQUAL_T,
    EQUAL_T, EQUAL_EQUAL_T,
    GREATER_T, GREATER_EQUAL_T, GLOBAL_T,
    LESS_T, LESS_EQUAL_T,
    // Literals.
    IDENTIFIER_T, STRING_T, NUMBER_T,
    // Keywords.
    AND_T, ELSE_T, FALSE_T,
    FOR_T, FUN_T, IF_T, NONE_T, OR_T, LEN_T,
    PRINT_T, PRINTLN_T, RETURN_T,
    TRUE_T, TYPE_T, VAR_T, WHILE_T,
    DEF_T, TIME_T,

    ERROR_T, EOF_T
} TokenType;

// This struct responsible for representing tokens.
// Copied & modified from cLox code by Robert Nystrom
typedef struct {
    TokenType type;
    const char* code;
    int length;
    int line;
} Token;

// This struct responsible for tokenizers.
// Copied & modified from cLox code by Robert Nystrom
typedef struct {
    const char* start;
    const char* current_char;
    int line;
} Tokenizer;

void initTokenizer(Tokenizer* tokenizer, const char* source);
Token nextToken(Tokenizer* tokenizer);

#endif //CJLANG_TOKEN_H
