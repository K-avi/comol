#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "lines.h"
#include "value.h"
#include "vm.h"

int main(int argc, const char* argv[]) {

  initVM();

  Chunk chunk;
  initChunk(&chunk, 250);

  int constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 202);
  writeChunk(&chunk, constant, 202);

  writeChunk(&chunk, OP_RETURN, 203);

  writeChunk(&chunk, OP_CONSTANT_LONG, 204);
  writeChunk(&chunk, 0, 204);
  writeChunk(&chunk, 0, 204);
  writeChunk(&chunk, 1, 204);
  chunk.constants.count++;
  chunk.constants.values[1]=3;


  writeChunk(&chunk, OP_RETURN, 207);
  //linesDump( chunk.lineCounter);
  disassembleChunk(&chunk, "test chunk");
  

  interpret(&chunk);
  freeVM();
  
  freeChunk(&chunk);
  freeLines((chunk.lineCounter));
  
  return 0;
}