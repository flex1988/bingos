#ifndef __ORDERED_ARRAY_H__
#define __ORDERED_ARRAY_H__

#include <types.h>

typedef void* type_t;

typedef int8_t (*comparer_t)(type_t, type_t);

typedef struct {
    type_t* array;
    uint32_t size;
    uint32_t max;
    comparer_t comparer;
} ordered_array_t;

int8_t default_comparer(type_t a, type_t b);

ordered_array_t create_ordered_array(uint32_t max, comparer_t comparer);
void destroy_ordered_array(ordered_array_t* array);
void insert_ordered_array(type_t item, ordered_array_t* array);
type_t lookup_ordered_array(uint32_t i, ordered_array_t* array);
void remove_ordered_array(uint32_t i, ordered_array_t* array);
ordered_array_t place_ordered_array(void* addr, uint32_t max, comparer_t comparer);
#endif
