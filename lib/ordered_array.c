#include "lib/ordered_array.h"
#include "mm/kheap.h"

int8_t default_comparer(type_t a, type_t b) { return *(uint32_t*)a - *(uint32_t*)b; }

ordered_array_t create_ordered_array(uint32_t max, comparer_t comparer) {
    ordered_array_t arr;
    arr.array = (type_t*)kmalloc(sizeof(type_t) * max);
    arr.size = 0;
    arr.max = max;
    arr.comparer = comparer;
    return arr;
}

void destroy_ordered_array(ordered_array_t* arr) {
    ;//kfree(arr->array);
    //kfree(arr);
}

void insert_ordered_array(type_t item, ordered_array_t* arr) {
    if (arr->size == arr->max)
        return;
    uint32_t i = 0;
    while (arr->comparer(arr->array[i], item) < 0) i++;

    if (i == arr->size) {
        arr->array[i] = item;
    } else {
        uint32_t j;
        for (j = i; j < arr->size; j++) {
            arr->array[j + 1] = arr->array[j];
        }
        arr->array[i] = item;
    }

    arr->size++;
}

type_t lookup_ordered_array(uint32_t i, ordered_array_t* arr) { return arr->array[i]; }

void remove_ordered_array(uint32_t i, ordered_array_t* arr) {
    uint32_t j;
    for (j = i; j < arr->size; j++) {
        arr->array[j] = arr->array[j + 1];
    }
    arr->size--;
}

ordered_array_t place_ordered_array(void* addr, uint32_t max, comparer_t comparer) {
    ordered_array_t arr;
    arr.array = (type_t*)addr;
    memset(arr.array, 0, max * sizeof(type_t));
    arr.size = 0;
    arr.max = max;
    arr.comparer = comparer;
    return arr;
}
