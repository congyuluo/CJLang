//
// Created by Congyu Luo on 9/28/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "token.h"
#include "imports.h"
#include "debugTools.h"
#include "object.h"
#include "makeString.h"

#define OPERAND_STACK_LIMIT 8

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
    Table function_addrs;
    Table function_operands;
    Value operand_stack[OPERAND_STACK_LIMIT];
    int operand_stack_index;
    Value* operand_stackTop;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

// Declare tokenizer instance
Tokenizer tokenizer;

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk *compilingChunk;

static void initParser() {
    parser.hadError = false;
    parser.panicMode = false;

    initTable(&parser.function_addrs);
    initTable(&parser.function_operands);
    parser.operand_stack_index = 0;
    parser.operand_stackTop = &parser.operand_stack[0];
}

static Chunk *currentChunk() {
    return compilingChunk;
}

static void errorAt(Token *token, const char *message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == EOF_T) {
        fprintf(stderr, " at end");
    } else if (token->type == ERROR_T) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->code);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char *message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char *message) {
    errorAt(&parser.current, message);
}

static void stackPush(Value value) {
    if (parser.operand_stack_index >= OPERAND_STACK_LIMIT){
        errorAtCurrent("Operand stack limit reached.");
    }

    *parser.operand_stackTop = value;
    parser.operand_stackTop++;
    parser.operand_stack_index++;
}

static Value stackPop() {
    if (parser.operand_stack_index <= 0){
        errorAtCurrent("Stack bottom reached.");
    }

    parser.operand_stack_index--;
    parser.operand_stackTop--;
    return *parser.operand_stackTop;
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = nextToken(&tokenizer);
#ifdef COMPILE_SHOW_TOKEN
        if (parser.previous.type != EOF_T) printToken(parser.current);
#endif
        if (parser.current.type != ERROR_T) break;

        errorAtCurrent(parser.current.code);
    }
}

static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    chunkAdd(currentChunk(), byte);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void emitConstant(Value value) {
    emitByte(OP_CONSTANT);
    chunkAddConstant(currentChunk(), value);
}

static void emitBackJump(OpCode jumpOp, uint16_t address) {
    Chunk* chunk = currentChunk();
    chunkAdd(chunk, jumpOp);
    chunkAdd(chunk, (address >> 8) & 0xff);
    chunkAdd(chunk, address & 0xff);
}

static int emitForwardJump(OpCode jumpOp) {
    Chunk* chunk = currentChunk();
    chunkAdd(chunk, jumpOp);
    int curr_index = chunk->current_index;
    chunkAdd(chunk, 0xff);
    chunkAdd(chunk, 0xff);
    return curr_index;
}

static void patchForwardJump(int patchAddr) {
    Chunk* chunk = currentChunk();
    int curr_addr = chunk->current_index;
    chunk->bytecode_array[patchAddr] = (curr_addr >> 8) & 0xff;
    chunk->bytecode_array[patchAddr + 1] = curr_addr & 0xff;
}

static void endCompiler() {
    emitReturn();
}

static void expression();

static ParseRule *getRule(TokenType type);

static void parsePrecedence(Precedence precedence);

static void statement();

static void functionCall();

static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence) (rule->precedence + 1));

    switch (operatorType) {
        case PLUS_T:
        case PLUS_EQUAL_T:
            emitByte(OP_ADD);
            break;
        case MINUS_T:
        case MINUS_EQUAL_T:
            emitByte(OP_SUBTRACT);
            break;
        case STAR_T:
        case STAR_EQUAL_T:
            emitByte(OP_MULTIPLY);
            break;
        case SLASH_T:
        case SLASH_EQUAL_T:
            emitByte(OP_DIVIDE);
            break;
        case CARET_T:
        case CARET_EQUAL_T:
            emitByte(OP_EXPONENT);
            break;
        case MOD_T:
        case MOD_EQUAL_T:
            emitByte(OP_MOD);
            break;
        case BANG_EQUAL_T:    emitBytes(OP_EQUAL, OP_NOT); break;
        case EQUAL_EQUAL_T:   emitByte(OP_EQUAL); break;
        case GREATER_T:       emitByte(OP_GREATER); break;
        case GREATER_EQUAL_T: emitBytes(OP_LESS, OP_NOT); break;
        case LESS_T:          emitByte(OP_LESS); break;
        case LESS_EQUAL_T:    emitBytes(OP_GREATER, OP_NOT); break;
        default:
            return; // Unreachable.
    }
}

static void grouping() {
    expression();
    consume(RIGHT_PAREN_T, "Expect ')' after expression.");
}

static void number() {
    double value = strtod(parser.previous.code, NULL);
    emitConstant(MAKE_NUMBER(value));
}

static void string() {
    emitConstant(makeStrValue(copyString(parser.previous.code + 1, parser.previous.length - 2), parser.previous.length-2));
}

static void boolTrue() {
    emitConstant(MAKE_BOOL(true));
}

static void boolFalse() {
    emitConstant(MAKE_BOOL(false));
}

static void none() {
    emitConstant(MAKE_NONE);
}

static void and_op() {
    int patch = emitForwardJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    parsePrecedence(PREC_AND);
    patchForwardJump(patch);
}

static void or_op() {
    int patch = emitForwardJump(OP_JUMP_IF_TRUE);
    emitByte(OP_POP);
    parsePrecedence(PREC_OR);
    patchForwardJump(patch);
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType) {
        case MINUS_T:
            emitByte(OP_NEGATE);
            break;
        default:
            return; // Unreachable.
    }
}

static void getIdentifier() {
    Value identifierName = makeStrValue(copyString(parser.previous.code, parser.previous.length), parser.previous.length);
    emitByte(OP_GET_VAR);
    chunkAddConstant(currentChunk(), identifierName);
}

static void funcPrefixCall() {
    functionCall();
    emitByte(OP_RV_POP);
}

static void identifier() {
    Value identifierName = makeStrValue(copyString(parser.previous.code, parser.previous.length), parser.previous.length);
    Value temp;
    if (tableGet(&parser.function_addrs, identifierName, &temp)){
        funcPrefixCall();
    } else {
        getIdentifier();
    }
}

static void typeFun() {
    consume(LEFT_PAREN_T, "Expect opening parentheses.");
    parsePrecedence(PREC_CALL);
    consume(RIGHT_PAREN_T, "Expect closing parentheses.");
    emitByte(OP_GET_TYPE);
}

static void lenFun() {
    consume(LEFT_PAREN_T, "Expect opening parentheses.");
    parsePrecedence(PREC_CALL);
    consume(RIGHT_PAREN_T, "Expect closing parentheses.");
    emitByte(OP_GET_LEN);
}

static void timeFun() {
    consume(LEFT_PAREN_T, "Expect opening parentheses.");
    consume(RIGHT_PAREN_T, "Expect closing parentheses.");
    emitByte(OP_GET_TIME);
}

ParseRule rules[] = {
        [LEFT_PAREN_T]    = {grouping, NULL, PREC_CALL},
        [RIGHT_PAREN_T]   = {NULL, NULL, PREC_NONE},
        [LEFT_BRACE_T]    = {NULL, NULL, PREC_NONE}, // [big]
        [RIGHT_BRACE_T]   = {NULL, NULL, PREC_NONE},
        [COMMA_T]         = {NULL, NULL, PREC_NONE},
        [DOT_T]           = {NULL, NULL, PREC_NONE},
        [MINUS_T]         = {unary, binary, PREC_TERM},
        [MINUS_EQUAL_T]         = {NULL, NULL, PREC_TERM},
        [PLUS_T]          = {NULL, binary, PREC_TERM},
        [PLUS_EQUAL_T]          = {NULL, NULL, PREC_TERM},
        [SEMICOLON_T]     = {NULL, NULL, PREC_NONE},
        [SLASH_T]         = {NULL, binary, PREC_FACTOR},
        [SLASH_EQUAL_T]         = {NULL, NULL, PREC_FACTOR},
        [STAR_T]          = {NULL, binary, PREC_FACTOR},
        [STAR_EQUAL_T]          = {NULL, NULL, PREC_FACTOR},
        [CARET_T]          = {NULL, binary, PREC_FACTOR},
        [CARET_EQUAL_T]          = {NULL, NULL, PREC_FACTOR},
        [MOD_T]          = {NULL, binary, PREC_FACTOR},
        [MOD_EQUAL_T]          = {NULL, NULL, PREC_FACTOR},
        [BANG_T]          = {NULL, NULL, PREC_NONE},
        [BANG_EQUAL_T]    = {NULL, binary, PREC_EQUALITY},
        [EQUAL_T]         = {NULL, NULL, PREC_NONE},
        [EQUAL_EQUAL_T]   = {NULL, binary, PREC_EQUALITY},
        [GREATER_T]       = {NULL, binary, PREC_COMPARISON},
        [GREATER_EQUAL_T] = {NULL, binary, PREC_COMPARISON},
        [GLOBAL_T]        = {NULL, NULL, PREC_NONE},
        [LESS_T]          = {NULL, binary, PREC_COMPARISON},
        [LESS_EQUAL_T]    = {NULL, binary, PREC_COMPARISON},
        [IDENTIFIER_T]    = {identifier, NULL, PREC_NONE},
        [STRING_T]        = {string, NULL, PREC_NONE},
        [NUMBER_T]        = {number, NULL, PREC_NONE},
        [AND_T]           = {NULL, and_op, PREC_AND},
        [ELSE_T]          = {NULL, NULL, PREC_NONE},
        [FALSE_T]         = {boolFalse, NULL, PREC_NONE},
        [FOR_T]           = {NULL, NULL, PREC_NONE},
        [FUN_T]           = {NULL, NULL, PREC_NONE},
        [IF_T]            = {NULL, NULL, PREC_NONE},
        [NONE_T]           = {none, NULL, PREC_NONE},
        [OR_T]            = {NULL, or_op, PREC_OR},
        [LEN_T]            = {lenFun, NULL, PREC_NONE},
        [PRINT_T]         = {NULL, NULL, PREC_NONE},
        [RETURN_T]        = {NULL, NULL, PREC_NONE},
        [TRUE_T]          = {boolTrue, NULL, PREC_NONE},
        [TYPE_T]          = {typeFun, NULL, PREC_NONE},
        [VAR_T]           = {NULL, NULL, PREC_NONE},
        [WHILE_T]         = {NULL, NULL, PREC_NONE},
        [DEF_T]         = {NULL, NULL, PREC_NONE},
        [TIME_T]         = {timeFun, NULL, PREC_NONE},
        [ERROR_T]         = {NULL, NULL, PREC_NONE},
        [EOF_T]           = {NULL, NULL, PREC_NONE},
};

static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void printStatement() {
    advance();
    expression();
    consume(SEMICOLON_T, "Expect end of statement.");
    emitByte(OP_PRINT);
}

static void printlnStatement() {
    advance();
    expression();
    consume(SEMICOLON_T, "Expect end of statement.");
    emitByte(OP_PRINTLN);
}

static void assignIdentidier(bool spec_global) {
    advance();
    Value identifierName = makeStrValue(copyString(parser.previous.code, parser.previous.length), parser.previous.length);
    if (parser.current.type == EQUAL_T){
        consume(EQUAL_T, "Expect assignment to identifier.");
        expression();
        consume(SEMICOLON_T, "Expect end of statement.");
        if (spec_global){
            emitByte(OP_SET_GLOBAL);
        } else {
            emitByte(OP_SET_VAR);
        }
        chunkAddConstant(currentChunk(), identifierName);
    } else {
        switch (parser.current.type) {
            case MINUS_EQUAL_T:
            case PLUS_EQUAL_T:
            case SLASH_EQUAL_T:
            case STAR_EQUAL_T:
            case CARET_EQUAL_T:
            case MOD_EQUAL_T:
                advance();
                emitByte(OP_GET_VAR);
                chunkAddConstant(currentChunk(), identifierName);
                binary();
                consume(SEMICOLON_T, "Expect end of statement.");
                if (spec_global){
                    emitByte(OP_SET_GLOBAL);
                } else {
                    emitByte(OP_SET_VAR);
                }
                chunkAddConstant(currentChunk(), identifierName);
                break;
            default:
                errorAtCurrent("Expect assignment to identifier.");
                break;
        }
    }
}

static void group() {
    advance();
    while (!(parser.current.type == RIGHT_BRACE_T || parser.current.type == EOF_T)) {
        statement();
    }
    consume(RIGHT_BRACE_T, "Expect closing brace after group.");
}

static void ifStatement() {
    advance();
    consume(LEFT_PAREN_T, "Expect opening parenthesis.");
    // Evaluate condition
    expression();
    consume(RIGHT_PAREN_T, "Expect closing parenthesis.");
    // Emit exit jump
    int patch = emitForwardJump(OP_JUMP_IF_FALSE_DISCARD);
    // Compile statement body
    statement();
    if (parser.current.type == ELSE_T) {
        advance();
        int else_patch = emitForwardJump(OP_JUMP);
        patchForwardJump(patch);
        statement();
        patchForwardJump(else_patch);
    } else {
        patchForwardJump(patch);
    }
}

static void whileStatement() {
    advance();
    consume(LEFT_PAREN_T, "Expect opening parenthesis.");
    // Store condition evaluation address
    int eval_address = currentChunk()->current_index;
    expression();
    consume(RIGHT_PAREN_T, "Expect closing parenthesis.");
    // Jump based on evaluated condition
    int patch = emitForwardJump(OP_JUMP_IF_FALSE_DISCARD);
    // Statement body
    statement();
    // Loopback
    emitBackJump(OP_JUMP, eval_address);
    // Patch exit condition address
    patchForwardJump(patch);
}

static void forStatement() {
    advance();
    consume(LEFT_PAREN_T, "Expect opening parenthesis.");
    int condition_addr = currentChunk()->current_index;
    // Condition
    expression();
    consume(SEMICOLON_T, "Expect ';'.");
    int exit_patch = emitForwardJump(OP_JUMP_IF_FALSE_DISCARD);
    int body_patch = emitForwardJump(OP_JUMP);
    // Increment operation
    int increment_addr = currentChunk()->current_index;
    statement();
    emitBackJump(OP_JUMP, condition_addr);
    consume(RIGHT_PAREN_T, "Expect closing parenthesis.");
    // Body statement
    patchForwardJump(body_patch);
    statement();
    // Evaluate back jump.
    emitBackJump(OP_JUMP, increment_addr);
    patchForwardJump(exit_patch);
}

static void defineStatement() {
    if (parser.operand_stack_index != 0) {
        errorAtCurrent("Internal failure, operand stack not empty.");
    }
    advance();
    if (parser.current.type != IDENTIFIER_T) {
        errorAtCurrent("Require function identifier.");
    }
    // Emit jump
    int end_function = emitForwardJump(OP_JUMP);
    // Add function to function table
    advance();
    Value functionName = makeStrValue(copyString(parser.previous.code, parser.previous.length), parser.previous.length);
    Value functionAddr = MAKE_NUMBER(currentChunk()->current_index);
    if (!tableSet(&parser.function_addrs, functionName, functionAddr)) {
        errorAtCurrent("Name has already been defined as function.");
    }
    // Parse operands
    consume(LEFT_PAREN_T, "Expect opening parenthesis.");
    while (parser.current.type != RIGHT_PAREN_T) {
        if (parser.current.type != IDENTIFIER_T) {
            errorAtCurrent("Expect identifiers.");
        }
        advance();
        Value operandName = makeStrValue(copyString(parser.previous.code, parser.previous.length), parser.previous.length);
        stackPush(operandName);
        if (parser.current.type != RIGHT_PAREN_T) {
            consume(COMMA_T, "Expect comma.");
        }
    }
    consume(RIGHT_PAREN_T, "");
    // Up Scope
    emitByte(OP_UP_SCOPE);
    // Collect operands & assign to identifiers.
    int operand_count = parser.operand_stack_index;
    // Record number of operands required.
    tableSet(&parser.function_operands, functionName, MAKE_NUMBER(operand_count));
    for (int i=0; i<operand_count; i++) {
        emitByte(OP_ASSIGN_LOCAL);
        emitByte(i + 1);
        chunkAddConstant(currentChunk(), stackPop());
    }
    consume(LEFT_BRACE_T, "Expect opening brace.");
    while (parser.current.type != RIGHT_BRACE_T) {
        // Parse statement body
        statement();
    }
    consume(RIGHT_BRACE_T, "Expect closing brace.");
    // Default return statement
    emitConstant(MAKE_NONE);
    emitByte(OP_RETURN);
    // Patch jump
    patchForwardJump(end_function);
}

static void functionCall() {
    Value functionName = makeStrValue(copyString(parser.previous.code, parser.previous.length), parser.previous.length);
    consume(LEFT_PAREN_T, "Expect opening parenthesis.");
    // Collect required number of operands
    Value operands_required;
    if (!tableGet(&parser.function_operands, functionName, &operands_required)){
        errorAtCurrent("Internal error, cannot find required number of operands for function call.");
    }
    int num_operands_given = 0;
    // Parse operands
    while (parser.current.type != RIGHT_PAREN_T) {
        expression();
        // Expect comma
        if (parser.current.type != RIGHT_PAREN_T) {
            consume(COMMA_T, "Expect comma.");
        }
        num_operands_given++;
    }
    if (num_operands_given != operands_required.content.number_value){
        errorAtCurrent("Incorrect number of operands for function call.");
    }
    consume(RIGHT_PAREN_T, "");
    emitByte(OP_RA_PUSH);
    chunkAddConstant(currentChunk(), MAKE_NUMBER(currentChunk()->current_index+4));
    Value functionAddr;
    tableGet(&parser.function_addrs, functionName, &functionAddr);
    emitBackJump(OP_JUMP, functionAddr.content.number_value);
}

static void returnStatement() {
    advance();
    if (parser.current.type == SEMICOLON_T) { // Return None
        emitConstant(MAKE_NONE);
    } else {
        expression();
    }
    consume(SEMICOLON_T, "Expect end of statement.");
    emitByte(OP_RETURN);
}

static void statement() {
    TokenType curr_statement = parser.current.type;
    if (curr_statement == IDENTIFIER_T) {
        Value functionName = makeStrValue(copyString(parser.current.code, parser.current.length), parser.current.length);
        Value temp;
        if (tableGet(&parser.function_addrs, functionName, &temp)){
            advance();
            functionCall();
            consume(SEMICOLON_T, "Expect end of statement.");
            return;
        }
    }
    switch (curr_statement) {
        case PRINT_T:
            printStatement(); break;
        case PRINTLN_T:
            printlnStatement(); break;
        case IDENTIFIER_T:
            assignIdentidier(false); break;
        case GLOBAL_T:
            advance();
            assignIdentidier(true); break;
        case LEFT_BRACE_T:
            group(); break;
        case IF_T:
            ifStatement(); break;
        case WHILE_T:
            whileStatement(); break;
        case FOR_T:
            forStatement(); break;
        case DEF_T:
            defineStatement(); break;
        case RETURN_T:
            returnStatement(); break;
        default:
            errorAtCurrent("Invalid statement type."); break;
    }
}

bool compile(const char *source, Chunk *chunk) {
    initTokenizer(&tokenizer, source);

    compilingChunk = chunk;

    initParser();

    advance();
    while (parser.current.type != EOF_T) {
        statement();
    }
    consume(EOF_T, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}
