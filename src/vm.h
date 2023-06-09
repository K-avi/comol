#ifndef comol_vm_h
#define comol_vm_h

#include "chunk.h"
#include "value.h"
#define STACK_DEFAULT 256 

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct {
  Chunk* chunk;
  uint8_t* ip;

  unsigned stack_size;
  Value* stack;
  Value* stackTop;
} VM;

void initVM();
void freeVM();

InterpretResult interpret(const char* source,  u_int32_t line_num);
void push(Value value);
Value pop();
extern VM vm;


#endif