#include "lines.h"
#include "common.h"
#include "memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>



void initLines( Lines* lineCounter, uint32_t capa){
    /*
    not tested ; 
    initialises an already allocated lineCounter. 
    */
    if(!lineCounter) return;
    if(lineCounter->lines) free(lineCounter->lines);
    lineCounter->lines= calloc(capa, sizeof(u_int32_t));

    if(!lineCounter->lines){
        
    }

    lineCounter->capacity=capa;
   
}//tested ok

void setLines( Lines * lineCounter, u_int32_t new_capa){
    /*
    sets the line array in a lineCounter to the capacity passed as argument;
    and sets every element in it to zero. Should only be called once at init of array
    
    checks for allocation failure
    */
    if(!lineCounter) return;
    if(lineCounter->lines) free(lineCounter->lines);

    lineCounter->lines= calloc(new_capa, sizeof(u_int32_t));

    if(!lineCounter->lines){
        fprintf(stderr,"error on allocation of lineCounter at %p\n", (void*)lineCounter);
        exit(-1);
    }
}// tested ok; useless though

void addLine( Lines * line, unsigned start_index){
  /*
  increments the instruction counter to every index after start index 

  checks for null ptr and such
  */
  if(!line) return;
  if( (!line->lines) || line->capacity< start_index) return;

  for(uint32_t i= start_index; i<line->capacity; i++){
    line->lines[i]++;
    //printf("curval %u\n", line->lines[i]);
  }
}//tested  ok

int getLine( Lines* lineCounter, unsigned instruct_index){
    /*
    does a binary search to figure out the line where an instruction was written on 

    it does so by checking if the instruction_index is more than the last instruction 
    of te previous line and less than the last one on the current line 

    (handles last/first line cases too)

    checks for nullptr
    */

    if(! lineCounter) {
        fprintf(stderr,"nullptr caught in getLine\n");
        return -1;
    }
    if(!lineCounter->lines) {
        fprintf(stderr,"nullptr caught in getLine\n");
        return -1;
    }
    
    int l = 0, r =lineCounter->capacity-1;
//printf("wth\n");
    while (l <= r){
        
        int m = l + (r-l)/2;

        //first line case 
        if( (uint32_t)m ==0 ){
            printf("exit path 0\n");
            return 1;
        }

        //last line case 
        if((uint32_t) m==lineCounter->capacity-1 && (instruct_index > lineCounter->lines[m]) ){
          printf("exit path 1 m %d  capa %d ii %d ll %d\n", m, lineCounter->capacity, instruct_index, lineCounter->lines[m]);
            return -1;
        }else if( (uint32_t) m==lineCounter->capacity-1){
            // printf("exit path i forgor\n");
            int nbdec=m;
            while( lineCounter->lines[m]== lineCounter->lines[nbdec] && nbdec>=1){
                nbdec--;
            }
            return nbdec+1;
        }

        //regular cases
        if(lineCounter->lines[m]==instruct_index){
            //printf("exit path 3\n");
          //  printf("%u %u ", instruct_index, lineCounter->lines[m]);
            int nbdec=m;
            while( lineCounter->lines[m]== lineCounter->lines[nbdec] && nbdec>=1){
                nbdec--;
            }
          
            return nbdec+1;
        }
        
        if ( (lineCounter->lines[m] > instruct_index) && (lineCounter->lines[m-1])<instruct_index ) 
        {
           //printf("exit path 4\n");
            int nbdec=m;
            while( lineCounter->lines[m]== lineCounter->lines[nbdec] && nbdec>=1){
                nbdec--;
            }
            return nbdec+1;
        }
        if (lineCounter->lines[m] <  instruct_index) l = m + 1;
        
        else r = m - 1;

       //printf("l=%d r=%d m=%d\n",l, r,m);
    }
    printf("exit path 5\n");
    return -1;
}// tested; doesnt work for some reason


void freeLines(Lines * lineCounter){

    if(!lineCounter) return ;

    free(lineCounter->lines);
    free(lineCounter);
}

void linesDump( Lines * lineCounter){
    if(!lineCounter )return;
    if(!lineCounter->lines) return;

    for(unsigned i=0; i<lineCounter->capacity; i++){
        printf("%u:%u ",i,lineCounter->lines[i]);
    }
}