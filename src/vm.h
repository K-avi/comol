#ifndef comol_vm_h
#define comol_vm_h

#include "chunk.h"
#include "value.h"
#define STACK_MAX 256 //make it dynamic at some point

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct {
  Chunk* chunk;
  uint8_t* ip;
  Value stack[STACK_MAX];
  Value* stackTop;
} VM;

void initVM();
void freeVM();

InterpretResult interpret(Chunk* chunk);

void push(Value value);
Value pop();


#endif