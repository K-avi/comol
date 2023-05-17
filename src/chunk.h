#ifndef comol_chunk_h
#define comol_chunk_h

#include "common.h"
#include "value.h"
#include <sys/types.h>


typedef enum {
  OP_CONSTANT,
  OP_RETURN,
} OpCode;


typedef struct {

  u_int32_t count;
  u_int32_t capacity;
  uint8_t* code;
  int* lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line) ;
int addConstant(Chunk* chunk, Value value);

#endif