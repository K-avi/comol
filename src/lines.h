#ifndef comol_lines_h
#define comol_lines_h

#include "common.h"

typedef struct{ 

  u_int32_t capacity;
  uint32_t * lines;
}Lines;
/*the idea of the Lines struct is to use a dynamic array to store the total of instruction at 
the nth line. To achieve this, each index of the array corresponds to a line. 

when increasing an index, every index after it is also increased bc it contains the sum of instructions
up to this point. 

This allows to retrieve the line where an instruction was in O(nlog) w a dichotomy search. 
the idea is that I start at the middle of the array n then if 
*/

/*
warning: line 1 is stored at index 0, line 2 index 1 and so on
*/

extern void initLines( Lines* lineCounter, uint32_t capa);

extern void setLines( Lines * lineCounter, u_int32_t new_capa);
extern void addLine( Lines * line, unsigned start_index);
extern int getLine( Lines* lineCounter, unsigned instruct_index);
extern void freeLines(Lines * lineCounter);
extern void linesDump( Lines * lineCounter);
#endif