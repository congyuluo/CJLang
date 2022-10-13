//
// Created by Congyu Luo on 9/26/22.
//
#include <string.h>

#include "token.h"
#include "imports.h"

// This function responsible for creating tokenizers.
// Copied & modified from cLox code by Robert Nystrom
void initTokenizer(Tokenizer* tokenizer, const char* source) {
    tokenizer->start = source;
    tokenizer->current_char = source;
    tokenizer->line = 1;
}

// This function responsible for creating tokens.
// Copied & modified from cLox code by Robert Nystrom
static Token makeToken(Tokenizer* tokenizer, TokenType type) {
    Token token;
    token.type = type;
    token.code = tokenizer->start;
    token.length = (int)(tokenizer->current_char - tokenizer->start);
    token.line = tokenizer->line;
    return token;
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}
static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd(Tokenizer* tokenizer) {
    return *tokenizer->current_char == '\0';
}

static char peekNextChar(Tokenizer* tokenizer) {
    return *tokenizer->current_char;
}

static bool match(Tokenizer* tokenizer, char expected) {
    if (isAtEnd(tokenizer)) return false;
    if (*tokenizer->current_char != expected) return false;
    tokenizer->current_char++;
    return true;
}

static char peekIndexAhead(Tokenizer* tokenizer, int index) {
    return tokenizer->current_char[index];
}

static char getNextChar(Tokenizer* tokenizer) {
    tokenizer->current_char++;
    return tokenizer->current_char[-1];
}

static void skipEmpty(Tokenizer* tokenizer){
    for (;;) {
        char current_char = peekNextChar(tokenizer);
        switch (current_char) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                getNextChar(tokenizer);
                break;
            default: return;
        }
    }
}

static TokenType checkKeyword(Tokenizer* tokenizer, int start, int length,
                              const char* rest, TokenType type) {
    if (tokenizer->current_char - tokenizer->start == start + length &&
        memcmp(tokenizer->start + start, rest, length) == 0) {
        return type;
    }

    return IDENTIFIER_T;
}

static TokenType identifierType(Tokenizer* tokenizer) {
    switch (tokenizer->start[0]) {
        case 'a': return checkKeyword(tokenizer, 1, 2, "nd", AND_T);
        case 'd': return checkKeyword(tokenizer, 1, 2, "ef", DEF_T);
        case 'e': return checkKeyword(tokenizer, 1, 3, "lse", ELSE_T);
        case 'F': return checkKeyword(tokenizer, 1, 4, "alse", FALSE_T);
        case 'f':
            if (tokenizer->current_char - tokenizer->start > 1) {
                switch (tokenizer->start[1]) {
                    case 'o': return checkKeyword(tokenizer, 2, 1, "r", FOR_T);
                    case 'u': return checkKeyword(tokenizer, 2, 1, "n", FUN_T);
                }
            }
            break;
        case 'G': return checkKeyword(tokenizer, 1, 5, "lobal", GLOBAL_T);
        case 'i': return checkKeyword(tokenizer, 1, 1, "f", IF_T);
        case 'N': return checkKeyword(tokenizer, 1, 3, "one", NONE_T);
        case 'o': return checkKeyword(tokenizer, 1, 1, "r", OR_T);
        case 'p': return checkKeyword(tokenizer, 1, 4, "rint", PRINT_T);
        case 'r': return checkKeyword(tokenizer, 1, 5, "eturn", RETURN_T);
        case 'l':
            if (tokenizer->current_char - tokenizer->start > 1) {
                switch (tokenizer->start[1]) {
                    case 'p': return checkKeyword(tokenizer, 2, 4, "rint", PRINTLN_T);
                    case 'e': return checkKeyword(tokenizer, 2, 1, "n", LEN_T);
                }
            }
            break;
        case 't':
            if (tokenizer->current_char - tokenizer->start > 1) {
                switch (tokenizer->start[1]) {
                    case 'y': return checkKeyword(tokenizer, 2, 2, "pe", TYPE_T);
                    case 'i': return checkKeyword(tokenizer, 2, 2, "me", TIME_T);
                }
            }
            break;
        case 'T': return checkKeyword(tokenizer, 1, 3, "rue", TRUE_T);
        case 'v': return checkKeyword(tokenizer, 1, 2, "ar", VAR_T);
        case 'w': return checkKeyword(tokenizer, 1, 4, "hile", WHILE_T);
    }
    return IDENTIFIER_T;
}

static Token identifier(Tokenizer* tokenizer) {
    while (isAlpha(peekNextChar(tokenizer)) || isDigit(peekNextChar(tokenizer))) getNextChar(tokenizer);
    return makeToken(tokenizer, identifierType(tokenizer));
}

static Token number(Tokenizer* tokenizer) {
    while (isDigit(peekNextChar(tokenizer))) getNextChar(tokenizer);

    // Look for a fractional part.
    if (peekNextChar(tokenizer) == '.' && isDigit(peekIndexAhead(tokenizer, 1))) {
        // Consume the ".".
        getNextChar(tokenizer);

        while (isDigit(peekNextChar(tokenizer))) getNextChar(tokenizer);
    }

    return makeToken(tokenizer, NUMBER_T);
}

static Token string(Tokenizer* tokenizer) {
    while (peekNextChar(tokenizer) != '"' && !isAtEnd(tokenizer)) {
        if (peekNextChar(tokenizer) == '\n') tokenizer->line++;
        getNextChar(tokenizer);
    }

    if (isAtEnd(tokenizer)) return makeToken(tokenizer, ERROR_T);

    // The closing quote.
    getNextChar(tokenizer);
    return makeToken(tokenizer, STRING_T);
}

Token nextToken(Tokenizer* tokenizer) {
    skipEmpty(tokenizer);
    tokenizer->start = tokenizer->current_char;

    if (isAtEnd(tokenizer)) {
        return makeToken(tokenizer, EOF_T);
    }

    char curr_char = getNextChar(tokenizer);
    
    if (isDigit(curr_char)) return number(tokenizer);
    if (isAlpha(curr_char)) return identifier(tokenizer);

    switch (curr_char) {
        case '(': return makeToken(tokenizer,LEFT_PAREN_T);
        case ')': return makeToken(tokenizer,RIGHT_PAREN_T);
        case '{': return makeToken(tokenizer,LEFT_BRACE_T);
        case '}': return makeToken(tokenizer,RIGHT_BRACE_T);
        case ';': return makeToken(tokenizer,SEMICOLON_T);
        case ',': return makeToken(tokenizer,COMMA_T);
        case '.': return makeToken(tokenizer,DOT_T);
        case '-':
            return makeToken(tokenizer, match(tokenizer, '=') ? MINUS_EQUAL_T : MINUS_T);
        case '+':
            return makeToken(tokenizer, match(tokenizer, '=') ? PLUS_EQUAL_T : PLUS_T);
        case '/':
            return makeToken(tokenizer, match(tokenizer, '=') ? SLASH_EQUAL_T : SLASH_T);
        case '*':
            return makeToken(tokenizer, match(tokenizer, '=') ? STAR_EQUAL_T : STAR_T);
        case '^':
            return makeToken(tokenizer, match(tokenizer, '=') ? CARET_EQUAL_T : CARET_T);
        case '!':
            return makeToken(tokenizer, match(tokenizer, '=') ? BANG_EQUAL_T : BANG_T);
        case '=':
            return makeToken(tokenizer,match(tokenizer, '=') ? EQUAL_EQUAL_T : EQUAL_T);
        case '<':
            return makeToken(tokenizer,match(tokenizer, '=') ? LESS_EQUAL_T : LESS_T);
        case '>':
            return makeToken(tokenizer,match(tokenizer, '=') ? GREATER_EQUAL_T : GREATER_T);
        case '%':
            return makeToken(tokenizer,match(tokenizer, '=') ? MOD_EQUAL_T : MOD_T);
        case '"': return string(tokenizer);
    }

    return makeToken(tokenizer, ERROR_T);
}
