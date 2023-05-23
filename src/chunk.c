#include "chunk.h"
#include "lines.h"
#include "memory.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


void initChunk(Chunk* chunk, u_int32_t line_num) {
   /*
   function from book ; modified for the lines stuff; 
   the line_num argument makes it possible to init the line counter array ; which is used 
   to retrieve lines n so on
   */
  if(!chunk) return;

  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;

  if(chunk->lineCounter){
   // freeLines(chunk->lineCounter);
  }
  
  chunk->lineCounter= (Lines* )malloc(sizeof(Lines));
  
  //to prevent reading from unititialised values
  chunk->lineCounter->lines=NULL; 
  chunk->lineCounter->capacity= 0;

  initLines(chunk->lineCounter, line_num);
  
  initValueArray(&chunk->constants);
}//tested

void writeChunk(Chunk* chunk, uint8_t byte, uint32_t instruct_line)  {
  /*
  function from book; edited to 
  include exercise from chapter 14; 
  */
  
  if(!chunk) return;
  
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
   
  }

  chunk->code[chunk->count] = byte;
  chunk->count++;
printf("in chunk instructline %u\n", instruct_line);
  addLine(chunk->lineCounter, instruct_line);

}// tested; ok when used right

int addConstant(Chunk* chunk, Value value) {
  /*
  function from book
  */
  /*if(!chunk){
     fprintf(stderr,"nullptr caught in addConstant at %p , %f\n", &addConstant, value); 
     return -1;
  }*/

  writeValueArray(&chunk->constants, value);
  return chunk->constants.count - 1;
}

void writeConstant(Chunk* chunk, Value value, int line) {
  /*
  adds the correct type of constant as a chunk. 
  adds value first; if it's >= to 256 writes the index of the constant as a 
  24 bits integer otherwise as an 8 bit one. 

  does so "cleanly" (by calling writechunk)
  */
 /* if(!chunk) {
   fprintf(stderr,"nullptr caught in writeConstant at %p\n", (void*) &writeConstant); 
   return;
  }*/
 
  int constant = addConstant(chunk, value);

  if(constant>=256){
    writeChunk(chunk, OP_CONSTANT_LONG, line);
    writeChunk(chunk, (constant>>16)&0xFF, line);
    writeChunk(chunk, (constant>>8)&0xFF, line);
    writeChunk(chunk, (constant)&0xFF, line);

  }else{
    writeChunk(chunk, OP_CONSTANT, line);
    writeChunk(chunk, constant, line);
  }


}//kinda tested; should be ok


void freeChunk(Chunk* chunk) {
  /*
  function from book; modified 
  */

  if(!chunk) return;

  uint32_t line_capa; 
  
  if(chunk->lineCounter) line_capa= chunk->lineCounter->capacity;
  else line_capa= 0;

  FREE_ARRAY(uint32_t, chunk->code, chunk->capacity);
  freeLines(chunk->lineCounter);
  freeValueArray(&chunk->constants);

  initChunk(chunk, line_capa);
}//reallocates memory for lines so check out for that