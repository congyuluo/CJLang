//
// Created by Congyu Luo on 9/25/22.
//

#include "debugTools.h"

static int singleOperandInstruction(Chunk* chunk, int index) {
    if (index + 1 >= chunk->current_index){
        printf("Chunk end reached, missing operand.");
        return index;
    }
    printf("  ^Operand| Value: ");
    printValue(chunk->constant_array.values[chunk->bytecode_array[index + 1]]);
    printf(" @[%d]\n", chunk->bytecode_array[index + 1]);
    return index + 1;
}

static int raPushInstruction(Chunk* chunk, int index) {
    if (index + 1 >= chunk->current_index){
        printf("Chunk end reached, missing operand.");
        return index;
    }
    int ra = chunk->constant_array.values[chunk->bytecode_array[index + 1]].content.number_value;
    printf("  ^Operand| Chunk Index:");
    printf("[%d]", ra);
    printf(" -> ");
    printOp(chunk->bytecode_array[ra]);
    return index + 1;
}

static int jumpInstruction(Chunk* chunk, int index) {
    if (index + 2 >= chunk->current_index){
        printf("Chunk end reached, missing operand.");
        return index;
    }
    uint16_t jumpto = (uint16_t)((chunk->bytecode_array[index + 1] << 8) | chunk->bytecode_array[index + 2]);
    printf("  ^Operand| Chunk Index:");
    printf("[%d]", jumpto);
    printf(" -> ");
    printOp(chunk->bytecode_array[jumpto]);
    return index + 2;
}

static int assignLocalInstruction(Chunk* chunk, int index) {
    if (index + 1 >= chunk->current_index){
        printf("Chunk end reached, missing operand.");
        return index;
    }
    printf("  ^Operand| Stack[-%d]\n", chunk->bytecode_array[index + 1]);
    printf("  ^Operand| Value: ");
    printValue(chunk->constant_array.values[chunk->bytecode_array[index + 2]]);
    printf(" @[%d]\n", chunk->bytecode_array[index + 2]);
    return index + 2;
}

void printChunk(Chunk* chunk){
    printf("--<CHUNK>--\n");
    // If there is no content in chunk
    if (chunk->size == 0){
        printf("[Empty Chunk]\n");
        return;
    }
    for (int i=0; i<chunk->current_index; i++){
        printf("# [%5d] | ", i);
        uint8_t current_code = chunk->bytecode_array[i];
        switch (current_code) {
            case OP_CONSTANT: {
                printf("OP_CONSTANT\n");
                i = singleOperandInstruction(chunk, i);
                break;
            }
            case OP_NULL: printf("OP_NULL\n"); break;
            case OP_TRUE: printf("OP_TRUE\n"); break;
            case OP_FALSE: printf("OP_FALSE\n"); break;
            case OP_POP: printf("OP_POP\n"); break;
            case OP_GET_TYPE: printf("OP_GET_TYPE\n"); break;
            case OP_GET_LEN: printf("OP_GET_LEN\n"); break;
            case OP_GET_TIME: printf("OP_GET_TIME\n"); break;
            case OP_GET_VAR: {
                printf("OP_GET_VAR\n");
                i = singleOperandInstruction(chunk, i);
                break;
            }
            case OP_SET_VAR: {
                printf("OP_SET_VAR\n");
                i = singleOperandInstruction(chunk, i);
                break;
            }
            case OP_ASSIGN_LOCAL: {
                printf("OP_ASSIGN_LOCAL\n");
                i = assignLocalInstruction(chunk, i);
                break;
            }
            case OP_SET_GLOBAL: {
                printf("OP_SET_GLOBAL\n");
                i = singleOperandInstruction(chunk, i);
                break;
            }
            case OP_UP_SCOPE: printf("OP_UP_SCOPE\n"); break;
            case OP_DOWN_SCOPE: printf("OP_DOWN_SCOPE\n"); break;
            case OP_EQUAL: printf("OP_EQUAL\n"); break;
            case OP_GREATER: printf("OP_GREATER\n"); break;
            case OP_LESS: printf("OP_LESS\n"); break;
            case OP_ADD: printf("OP_ADD\n"); break;
            case OP_SUBTRACT: printf("OP_SUBTRACT\n"); break;
            case OP_MULTIPLY: printf("OP_MULTIPLY\n"); break;
            case OP_DIVIDE: printf("OP_DIVIDE\n"); break;
            case OP_EXPONENT: printf("OP_EXPONENT\n"); break;
            case OP_MOD: printf("OP_MOD\n"); break;
            case OP_NOT: printf("OP_NOT\n"); break;
            case OP_NEGATE: printf("OP_NEGATE\n"); break;
            case OP_PRINT: printf("OP_PRINT\n"); break;
            case OP_PRINTLN: printf("OP_PRINTLN\n"); break;
            case OP_JUMP: {
                printf("OP_JUMP\n");
                i = jumpInstruction(chunk, i);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                printf("OP_JUMP_IF_FALSE\n");
                i = jumpInstruction(chunk, i);
                break;
            }
            case OP_JUMP_IF_FALSE_DISCARD: {
                printf("OP_JUMP_IF_FALSE_DISCARD\n");
                i = jumpInstruction(chunk, i);
                break;
            }
            case OP_JUMP_IF_TRUE: {
                printf("OP_JUMP_IF_TRUE\n");
                i = jumpInstruction(chunk, i);
                break;
            }
            case OP_LOOP: printf("OP_LOOP\n"); break;
            case OP_CALL: printf("OP_CALL\n"); break;
            case OP_RA_PUSH: {
                printf("OP_RA_PUSH\n");
                i = raPushInstruction(chunk, i);
                break;
            }
            case OP_RV_POP: printf("OP_RV_POP\n"); break;
            case OP_RETURN: printf("OP_RETURN\n"); break;
            default: {
                break;
            }
        }
    }
}

void printOp(uint8_t opCode) {
    printf("Op: [");
    switch (opCode) {
        case OP_CONSTANT: printf("OP_CONSTANT]\n"); break;
        case OP_NULL: printf("OP_NULL]\n"); break;
        case OP_TRUE: printf("OP_TRUE]\n"); break;
        case OP_FALSE: printf("OP_FALSE]\n"); break;
        case OP_POP: printf("OP_POP]\n"); break;
        case OP_GET_TYPE: printf("OP_GET_TYPE]\n"); break;
        case OP_GET_LEN: printf("OP_GET_LEN]\n"); break;
        case OP_GET_TIME: printf("OP_GET_TIME]\n"); break;
        case OP_GET_VAR: printf("OP_GET_VAR]\n"); break;
        case OP_SET_GLOBAL: printf("OP_SET_GLOBAL]\n"); break;
        case OP_SET_VAR: printf("OP_SET_VAR]\n"); break;
        case OP_ASSIGN_LOCAL: printf("OP_ASSIGN_LOCAL]\n"); break;
        case OP_UP_SCOPE: printf("OP_UP_SCOPE]\n"); break;
        case OP_DOWN_SCOPE: printf("OP_DOWN_SCOPE]\n"); break;
        case OP_EQUAL: printf("OP_EQUAL]\n"); break;
        case OP_GREATER: printf("OP_GREATER]\n"); break;
        case OP_LESS: printf("OP_LESS]\n"); break;
        case OP_ADD: printf("OP_ADD]\n"); break;
        case OP_SUBTRACT: printf("OP_SUBTRACT]\n"); break;
        case OP_MULTIPLY: printf("OP_MULTIPLY]\n"); break;
        case OP_DIVIDE: printf("OP_DIVIDE]\n"); break;
        case OP_EXPONENT: printf("OP_EXPONENT]\n"); break;
        case OP_MOD: printf("OP_MOD]\n"); break;
        case OP_NOT: printf("OP_NOT]\n"); break;
        case OP_NEGATE: printf("OP_NEGATE]\n"); break;
        case OP_PRINT: printf("OP_PRINT]\n"); break;
        case OP_PRINTLN: printf("OP_PRINTLN]\n"); break;
        case OP_JUMP: printf("OP_JUMP]\n"); break;
        case OP_JUMP_IF_FALSE: printf("OP_JUMP_IF_FALSE]\n"); break;
        case OP_JUMP_IF_FALSE_DISCARD: printf("OP_JUMP_IF_FALSE_DISCARD]\n"); break;
        case OP_JUMP_IF_TRUE: printf("OP_JUMP_IF_TRUE]\n"); break;
        case OP_LOOP: printf("OP_LOOP]\n"); break;
        case OP_CALL: printf("OP_CALL]\n"); break;
        case OP_RA_PUSH: printf("OP_RA_PUSH]\n"); break;
        case OP_RV_POP: printf("OP_RV_POP]\n"); break;
        case OP_RETURN: printf("OP_RETURN]\n"); break;
        default: {
            break;
        }
    }
}

void printStack(VM* vm) {
    if (vm->stack_index == 0) {
        printf("[Stack Empty]\n");
        return;
    }
    printf("[");
    for (int i=0; i<vm->stack_index; i++) {
        printf("#%3d: ", i);
        debugPrintValue(vm->stack[i]);
        if (i != vm->stack_index - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

void printToken(Token token) {
    switch (token.type) {
        case LEFT_PAREN_T: printf("[LEFT_PAREN_T]"); break;
        case  RIGHT_PAREN_T: printf("[RIGHT_PAREN_T]"); break;
        case LEFT_BRACE_T: printf("[LEFT_BRACE_T]"); break;
        case  RIGHT_BRACE_T: printf("[RIGHT_BRACE_T]"); break;
        case COMMA_T: printf("[COMMA_T]"); break;
        case  DOT_T: printf("[DOT_T]"); break;
        case  MINUS_T: printf("[MINUS_T]"); break;
        case  MINUS_EQUAL_T: printf("[MINUS_EQUAL_T]"); break;
        case  PLUS_T: printf("[PLUS_T]"); break;
        case  PLUS_EQUAL_T: printf("[PLUS_EQUAL_T]"); break;
        case SEMICOLON_T: printf("[SEMICOLON_T]"); break;
        case  SLASH_T: printf("[SLASH_T]"); break;
        case  SLASH_EQUAL_T: printf("[SLASH_EQUAL_T]"); break;
        case  STAR_T: printf("[STAR_T]"); break;
        case  STAR_EQUAL_T: printf("[STAR_EQUAL_T]"); break;
        case  CARET_T: printf("[CARET_T]"); break;
        case  CARET_EQUAL_T: printf("[CARET_EQUAL_T]"); break;
        case  MOD_T: printf("[MOD_T]"); break;
        case  MOD_EQUAL_T: printf("[MOD_EQUAL_T]"); break;
        case BANG_T: printf("[BANG_T]"); break;
        case  BANG_EQUAL_T: printf("[BANG_EQUAL_T]"); break;
        case EQUAL_T: printf("[EQUAL_T]"); break;
        case  EQUAL_EQUAL_T: printf("[EQUAL_EQUAL_T]"); break;
        case GREATER_T: printf("[GREATER_T]"); break;
        case  GREATER_EQUAL_T: printf("[GREATER_EQUAL_T]"); break;
        case  GLOBAL_T: printf("[GLOBAL_T]"); break;
        case LESS_T: printf("[LESS_T]"); break;
        case  LESS_EQUAL_T: printf("[LESS_EQUAL_T]"); break;
        case IDENTIFIER_T: printf("[IDENTIFIER_T]"); break;
        case  STRING_T: printf("[STRING_T]"); break;
        case  NUMBER_T: printf("[NUMBER_T]"); break;
        case AND_T: printf("[AND_T]"); break;
        case  ELSE_T: printf("[ELSE_T]"); break;
        case  FALSE_T: printf("[FALSE_T]"); break;
        case FOR_T: printf("[FOR_T]"); break;
        case  FUN_T: printf("[FUN_T]"); break;
        case  IF_T: printf("[IF_T]"); break;
        case  NONE_T: printf("[NONE_T]"); break;
        case  OR_T: printf("[OR_T]"); break;
        case  LEN_T: printf("[LEN_T]"); break;
        case PRINT_T: printf("[PRINT_T]"); break;
        case PRINTLN_T: printf("[PRINTLN_T]"); break;
        case  RETURN_T: printf("[RETURN_T]"); break;
        case TRUE_T: printf("[TRUE_T]"); break;
        case TYPE_T: printf("[TYPE_T]"); break;
        case  VAR_T: printf("[VAR_T]"); break;
        case  WHILE_T: printf("[WHILE_T]"); break;
        case  DEF_T: printf("[DEF_T]"); break;
        case  TIME_T: printf("[TIME_T]"); break;
        case ERROR_T: printf("[ERROR_T]"); break;
        case  EOF_T: printf("[EOF_T]"); break;
        default: printf("Unknown Token."); return;
    }
    printf(" %.*s\n", token.length, token.code);
}
