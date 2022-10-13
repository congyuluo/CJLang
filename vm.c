//
// Created by Congyu Luo on 9/25/22.
//
#include <string.h>
#include <math.h>

#include "vm.h"
#include "debugTools.h"
#include "object.h"
#include "memory.h"
#include "makeString.h"


static OperationResult runtimeError(VM* vm, char* message) {
    vm->hasError = true;
    fprintf(stderr, message);
    printf("\n");
    return RUNTIME_FAILURE;
}

void initVM(VM* vm, Chunk* chunk) {
    vm->chunk = chunk;
    vm->instruction_pointer = &vm->chunk->bytecode_array[0];
    vm->stack_index = 0;
    vm->ra_stack_index = 0;
    vm->local_index = 0;
    vm->stackTop = &vm->stack[0];
    vm->ra_stackTop = &vm->ra_stack[0];
    vm->hasError = false;
    vm->scope = 0;
    vm->returnValue = MAKE_NONE;
    initTable(&vm->globals);
}

void initLocal(Local* local, Value key, int index, int scope) {
    local->key = key;
    local->index = index;
    local->scope = scope;
}

static void stackPush(VM* vm, Value value) {
    if (vm->stack_index >= STACK_LIMIT){
        printf("Stack limit reached.");
        vm->hasError = true;
    }

    *vm->stackTop = value;
    vm->stackTop++;
    vm->stack_index++;
}

static Value stackPop(VM* vm) {
    if (vm->stack_index <= 0){
        printf("Return address bottom reached.");
        vm->hasError = true;
    }
    vm->stack_index--;
    vm->stackTop--;
    return *vm->stackTop;
}

static void raStackPush(VM* vm, Value value) {
    if (vm->ra_stack_index >= STACK_LIMIT){
        printf("Maximum recursion / function call reached.");
        vm->hasError = true;
    }

    *vm->ra_stackTop = value;
    vm->ra_stackTop++;
    vm->ra_stack_index++;
}

static Value raStackPop(VM* vm) {
    if (vm->ra_stack_index <= 0){
        printf("Stack bottom reached.");
        vm->hasError = true;
    }
    vm->ra_stack_index--;
    vm->ra_stackTop--;
    return *vm->ra_stackTop;
}

static Value stackPeek(VM* vm) {
    if (vm->stack_index <= 0){
        printf("Attempting to peek empty stack.");
        vm->hasError = true;
    }
    return vm->stackTop[-1];
}

static Chunk* currentChunk(VM* vm) {
    return vm->chunk;
}

static double exponent(double base, double exp) {
    double result = base;
    for (int i=0; i<exp - 1; i++){
        result *= base;
    }
    return result;
}

static bool getLocal(VM* vm, Value key, Value* v) {
    if (vm->local_index == 0 || key.type != OBJECT_STRING_TYPE){
        return false;
    }
    for (int i=vm->local_index - 1; i>=0; i--) {
        Local current_local = vm->locals[i];
        // Break if different scope has been reached
        if (current_local.scope < vm->scope) {
            return false;
        }
        // Compare key
        if (current_local.key.content.string_object == key.content.string_object){
            // Set v to value
            *v = vm->stack[current_local.index];
            return true;
        }
    }
    return false;
}

static void setLocal(VM* vm, Value key, Value v) {
    // Store location of value on vm's stack as a local
    Local local;
    initLocal(&local, key, vm->stack_index, vm->scope);
    // Push value to stack
    stackPush(vm, v);
    // Add new local to vm's local array
    vm->locals[vm->local_index] = local;
    // Increment local index
    vm->local_index++;
}

static void assignLocal(VM* vm, Value key, uint8_t offset) {
    // Store location of value on vm's stack as a local
    Local local;
    initLocal(&local, key, vm->stack_index - offset, vm->scope);
    // Add new local to vm's local array
    vm->locals[vm->local_index] = local;
    // Increment local index
    vm->local_index++;
}

static void cleanLocalsAtScope(VM* vm) {
    if (vm->scope == 0 || vm->local_index == 0){
        return;
    }
    int local_count = 0;
    for (int i = vm->local_index-1; i>=0; i--) {
        Local current_local = vm->locals[i];
        // Break if all locals in current scope has been deleted
        if (current_local.scope < vm->scope){
            break;
        }
        // Increment counter
        local_count++;
        // Pop local from stack
        stackPop(vm);
    }
    vm->local_index -= local_count;
}

static bool jump(VM* vm) {
#define READ_SHORT() (vm->instruction_pointer += 2, (uint16_t)((vm->instruction_pointer[-2] << 8) | vm->instruction_pointer[-1]))
    uint16_t offset = READ_SHORT();
    if (offset >= vm->chunk->current_index){
        return false;
    }
    vm->instruction_pointer = &vm->chunk->bytecode_array[offset];
    return true;
}

OperationResult run(VM* vm){
#ifdef RUNTIME_SHOW_EXECUTION
    int cycle_count = 0;
#endif

    for (;;) {
        if (vm->hasError) {
            return RUNTIME_FAILURE;
        }
        // Get current chunk
        Chunk* current_chunk = currentChunk(vm);
        // Get next instruction
        uint8_t curr_instruction = *vm->instruction_pointer++;

#ifdef RUNTIME_SHOW_EXECUTION
        cycle_count++;
        printf("\n[Cycle: %4d, Scope: %2d]->", cycle_count, vm->scope);
        printOp(curr_instruction);
        printStack(vm);
#endif

        switch (curr_instruction) {
            case OP_CONSTANT: {
                stackPush(vm, current_chunk->constant_array.values[*vm->instruction_pointer++]);
                break;
            }
            case OP_POP: {
                stackPop(vm);
                break;
            }
            case OP_GET_TYPE: {
                stackPush(vm, makeStrValue(strValueType(stackPop(vm)), 9));
                break;
            }
            case OP_GET_LEN: {
                Value v = stackPop(vm);
                if (v.type != OBJECT_STRING_TYPE) {
                    return runtimeError(vm, "Can only use len() on OBJ_STRING type.");
                }
                stackPush(vm, MAKE_NUMBER(v.content.string_object->length));
                break;
            }
            case OP_GET_TIME: {
                stackPush(vm, MAKE_NUMBER(time(NULL)));
                break;
            }
            case OP_GET_VAR: {
                // First get key value
                Value key_value = current_chunk->constant_array.values[*vm->instruction_pointer++];
                Value v1;
                // If not currently in global scope, prioritize search in local scope
                if (vm->scope > 0 && getLocal(vm, key_value, &v1)) {
                    stackPush(vm, v1);
                } else { // Else, search in global scope
                    Value v;
                    if (tableGet(&vm->globals, key_value, &v)){
                        stackPush(vm, v);
                    } else {
                        if (vm->scope == 0) {
                            printf("Variable with name '%.*s' does not exist in global scope.", key_value.content.string_object->length, key_value.content.string_object->cString);
                        } else {
                            printf("Variable with name '%.*s' does not exist in current scope or global scope.", key_value.content.string_object->length, key_value.content.string_object->cString);
                        }
                        return runtimeError(vm, "");
                    }
                }
                break;
            }
            case OP_SET_VAR: {
                Value key = current_chunk->constant_array.values[*vm->instruction_pointer++];
                if (vm->scope > 0) {
                    if (key.type != OBJECT_STRING_TYPE) {
                        return runtimeError(vm, "Key for variable must be string type");
                    }
                    setLocal(vm, key, stackPop(vm));
                } else {
                    tableSet(&vm->globals, key, stackPop(vm));
                }
                break;
            }
            case OP_ASSIGN_LOCAL: {
                uint8_t offset = *vm->instruction_pointer++;
                Value key = current_chunk->constant_array.values[*vm->instruction_pointer++];
                assignLocal(vm, key, offset);
                break;
            }
            case OP_SET_GLOBAL: {
                Value key = current_chunk->constant_array.values[*vm->instruction_pointer++];
                tableSet(&vm->globals, key, stackPop(vm));
                break;
            }
            case OP_UP_SCOPE: {
                vm->scope += 1;
                break;
            }
            case OP_DOWN_SCOPE: {
                cleanLocalsAtScope(vm);
                vm->scope -= 1;
                break;
            }
            case OP_EQUAL: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) {
                    stackPush(vm, MAKE_BOOL(false));
                } else {
                    switch (v1.type) {
                        case OBJECT_STRING_TYPE: stackPush(vm, MAKE_BOOL(v1.content.string_object == v2.content.string_object)); break;
                        case NUMBER_TYPE: stackPush(vm, MAKE_BOOL(v1.content.number_value == v2.content.number_value)); break;
                        case BOOL_TYPE: stackPush(vm, MAKE_BOOL(v1.content.bool_value == v2.content.bool_value)); break;
                        case NONE_TYPE: stackPush(vm, MAKE_BOOL(true)); break;
                        default: return runtimeError(vm, "Unsupported operand type.");
                    }
                }
                break;
            }
            case OP_GREATER: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) {
                    return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                } else if (v1.type != NUMBER_TYPE) {
                    return runtimeError(vm, "Cannot compare non-number values.");
                }
                stackPush(vm, MAKE_BOOL(v1.content.number_value > v2.content.number_value));
                break;
            }
            case OP_LESS: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) {
                    return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                } else if (v1.type != NUMBER_TYPE) {
                    return runtimeError(vm, "Cannot compare non-number values.");
                }
                stackPush(vm, MAKE_BOOL(v1.content.number_value < v2.content.number_value));
                break;
            }
            case OP_NOT: {
                Value v = stackPop(vm);
                if (v.type != BOOL_TYPE) {
                    return runtimeError(vm, "Cannot invert non-boolean values.");
                }
                stackPush(vm, MAKE_BOOL(!v.content.bool_value));
                break;
            }
            case OP_ADD: {
                    Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                    if (v1.type != v2.type) {
                        return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                    } else if ((v1.type != NUMBER_TYPE) && (v1.type != OBJECT_STRING_TYPE)) {
                        return runtimeError(vm, "Unsupported operand type.");
                    }
                    if (v1.type == NUMBER_TYPE) {
                        stackPush(vm, MAKE_NUMBER(v1.content.number_value + v2.content.number_value));
                    } else {
                        String_Object* a = v1.content.string_object;
                        String_Object* b = v2.content.string_object;
                        int length = a->length + b->length;
                        char* chars = ALLOCATE(char, length + 1);
                        memcpy(chars, a->cString, a->length);
                        memcpy(chars + a->length, b->cString, b->length);
                        chars[length] = '\0';
                        stackPush(vm, makeStrValue(chars, length));
                    }
                    break;
            }
            case OP_SUBTRACT: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                if (v1.type != NUMBER_TYPE) return runtimeError(vm, "Unsupported operand type.");

                stackPush(vm, MAKE_NUMBER(v1.content.number_value - v2.content.number_value));
                break;
            }
            case OP_MULTIPLY: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                if (v1.type != NUMBER_TYPE) return runtimeError(vm, "Unsupported operand type.");

                stackPush(vm, MAKE_NUMBER(v1.content.number_value * v2.content.number_value));
                break;
            }
            case OP_DIVIDE: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                if (v1.type != NUMBER_TYPE) return runtimeError(vm, "Unsupported operand type.");
                stackPush(vm, MAKE_NUMBER(v1.content.number_value / v2.content.number_value));
                break;
            }
            case OP_EXPONENT: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                if (v1.type != NUMBER_TYPE) return runtimeError(vm, "Unsupported operand type.");
                stackPush(vm, MAKE_NUMBER(exponent(v1.content.number_value, v2.content.number_value)));
                break;
            }
            case OP_MOD: {
                Value v2 = stackPop(vm); Value v1 = stackPop(vm);
                if (v1.type != v2.type) return runtimeError(vm, "Cannot perform binary operation on values of different types.");
                if (v1.type != NUMBER_TYPE) return runtimeError(vm, "Unsupported operand type.");
                stackPush(vm, MAKE_NUMBER(remainder(v1.content.number_value, v2.content.number_value)));
                break;
            }
            case OP_NEGATE: {
                Value v = stackPop(vm);
                if (v.type == BOOL_TYPE) {
                    stackPush(vm, MAKE_BOOL(!v.content.bool_value));
                } else if (v.type == NUMBER_TYPE) {
                    stackPush(vm, MAKE_NUMBER(-v.content.number_value));
                } else {
                    return runtimeError(vm, "Unsupported operand type.");
                }
                break;
            }
            case OP_PRINT: {
#ifdef RUNTIME_SHOW_EXECUTION
                printf("Output: ");
#endif
                printValue(stackPop(vm));
                break;
            }
            case OP_PRINTLN: {
#ifdef RUNTIME_SHOW_EXECUTION
                printf("Output: ");
#endif
                printValue(stackPop(vm));
                printf("\n");
                break;
            }
            case OP_JUMP: {
                if (!jump(vm)) return runtimeError(vm, "Jump address out of bound.");
                break;
            }
            case OP_JUMP_IF_FALSE: {
                Value v = stackPeek(vm);
                if (v.type != BOOL_TYPE) {
                    return runtimeError(vm, "Invalid jump condition, condition must be bool.");
                }
                if (!v.content.bool_value) {
                    if (!jump(vm)) return runtimeError(vm, "Jump address out of bound.");
                } else { // Skip jump address
                    vm->instruction_pointer += 2;
                }
                break;
            }
            case OP_JUMP_IF_FALSE_DISCARD: {
                Value v = stackPop(vm);
                if (v.type != BOOL_TYPE) {
                    return runtimeError(vm, "Invalid jump condition, condition must be bool.");
                }
                if (!v.content.bool_value) {
                    if (!jump(vm)) return runtimeError(vm, "Jump address out of bound.");
                } else { // Skip jump address
                    vm->instruction_pointer += 2;
                }
                break;
            }
            case OP_JUMP_IF_TRUE: {
                Value v = stackPeek(vm);
                if (v.type != BOOL_TYPE) {
                    return runtimeError(vm, "Invalid jump condition, condition must be bool.");
                }
                if (v.content.bool_value) {
                    if (!jump(vm)) return runtimeError(vm, "Jump address out of bound.");
                } else { // Skip jump address
                    vm->instruction_pointer += 2;
                }
                break;
            }
            case OP_RA_PUSH: {
                raStackPush(vm, current_chunk->constant_array.values[*vm->instruction_pointer++]);
                break;
            }
            case OP_RV_POP: {
                stackPush(vm, vm->returnValue);
                break;
            }
            case OP_RETURN: {
                if (vm->scope == 0) {
                    return RUNTIME_SUCCESS;
                } else { // Jump to return addr
                    // Pop return value
                    vm->returnValue = stackPop(vm);
                    // Clean up scope
                    cleanLocalsAtScope(vm);
                    vm->scope-=1;
                    vm->instruction_pointer = &vm->chunk->bytecode_array[(int) raStackPop(vm).content.number_value];
                }
                break;
            }
            default:
                return runtimeError(vm, "Unknown Opcode");
        }
#ifdef RUNTIME_SHOW_EXECUTION
        printf("->");
        printStack(vm);
#endif
    }
}