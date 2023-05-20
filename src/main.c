#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "lines.h"
#include "value.h"

int main(int argc, const char* argv[]) {

  Chunk chunk;
  initChunk(&chunk, 250);

  int constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 202);
  writeChunk(&chunk, constant, 202);

  writeChunk(&chunk, OP_RETURN, 203);

  //linesDump( chunk.lineCounter);
  disassembleChunk(&chunk, "test chunk");
  freeChunk(&chunk);
  freeLines((chunk.lineCounter));
  return 0;
}