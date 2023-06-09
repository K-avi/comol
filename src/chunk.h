#ifndef comol_chunk_h
#define comol_chunk_h

#include "common.h"
#include "value.h"
#include "lines.h"
#include <sys/types.h>


typedef enum {
  OP_CONSTANT,
  OP_CONSTANT_LONG,

  OP_NIL,
  OP_TRUE,
  OP_FALSE,

  OP_NOT,

  OP_EQUAL,
  OP_GREATER,
  OP_LESS,

  OP_RETURN,
  OP_NEGATE,
  
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
} OpCode;




typedef struct {

  uint32_t count;
  uint32_t capacity;
  uint8_t* code;
  
  Lines* lineCounter;
  ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk, uint32_t line_num);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, uint32_t line) ;
void writeConstant(Chunk* chunk, Value value, int line);
int addConstant(Chunk* chunk, Value value);

#endif