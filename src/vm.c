#include "chunk.h"
#include "common.h"
#include "memory.h"
#include "value.h"
#include "vm.h"
#include "compiler.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

//I WILL change that cuz I don't really like it
VM vm; 

static void resetStack() {
  vm.stackTop = vm.stack;
}

void initVM() {

    if(!vm.stack){
      vm.stack= malloc(STACK_DEFAULT*sizeof(Value));
    }
    vm.stack_size=STACK_DEFAULT;
    resetStack();
}



void freeVM() {
  free(vm.stack);
}

void push(Value value) {

  if(vm.stackTop- vm.stack >= vm.stack_size){
    unsigned top_index= vm.stackTop- vm.stack;
    vm.stack=GROW_ARRAY(Value, vm.stack, vm.stack_size, 2*vm.stack_size);
    vm.stack_size*=2;

    vm.stackTop= vm.stack +top_index;
  }
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static InterpretResult run() {
/*
   function from book ; need to modify it to support long constants    
*/
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
      double b = pop(); \
      double a = pop(); \
      push(a op b); \
    } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    

     printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk,
                           (int)(vm.ip - vm.chunk->code));
#endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {

      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_CONSTANT_LONG : {
        
        //wanted to do this cleanly w macro but had weird warning about evaluation order
        unsigned first8= (READ_BYTE() <<16) , mid8= (READ_BYTE())<<8, last8= READ_BYTE() ;
        unsigned final_index= first8 | mid8 | last8;

        Value constant= vm.chunk->constants.values[final_index];
        push(constant); 
        break;
      }

      case OP_NEGATE:  *(vm.stackTop-1)= -*(vm.stackTop-1); break;

      case OP_ADD:      BINARY_OP(+); break;
      case OP_SUBTRACT: BINARY_OP(-); break;
      case OP_MULTIPLY: BINARY_OP(*); break;
      case OP_DIVIDE:   BINARY_OP(/); break;
      
      
      case OP_RETURN: {
        printValue(pop());
        printf("\n");
        return INTERPRET_OK;
      }
    }
  }
#undef READ_CONSTANT
#undef READ_BYTE
#undef BINARY_OP
}//remember to add long constant support! 

InterpretResult interpret(const char* source, u_int32_t line_num) {
  /* book fn; need to modify to make initChunk work n stuff*/
  Chunk chunk;
  initChunk(&chunk, line_num);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;

}
