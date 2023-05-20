#ifndef comol_chunk_h
#define comol_chunk_h

#include "common.h"
#include "value.h"
#include "lines.h"
#include <sys/types.h>


typedef enum {
  OP_CONSTANT,
  OP_RETURN,
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
int addConstant(Chunk* chunk, Value value);

#endif