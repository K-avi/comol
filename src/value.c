#include <stdio.h>

#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {

  //if(!array) return;
  if(!array){
    perror("array is null\n");
  }
 // printf("writ %p , array-> cap , array -> cou \n", array);

  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
  }

  //printf("newvals array : %p , array-> cap %d, array -> cou %d\n", array, array->capacity, array->count);

  array->values[array->count] = value;
  array->count++;
}

void freeValueArray(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

void printValue(Value value) {
  printf("%g", value);
}
