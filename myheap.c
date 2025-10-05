#include "myheap.h"
#include <unistd.h>


#define HEAP_SIZE 10000     /* adjust as necessary */

static char heap[HEAP_SIZE];
static char *freep = heap;


char *alloc(int size) {
    if (freep + size > heap + HEAP_SIZE) {
        // Not enough space left
        return NULL;
    }
    char *p = freep;     // save pointer to current free space
    freep += size;       // move freep ahead by size
    return p;            // return pointer to start of allocated chunk
}


void free_all() {
    freep = heap;   // reset back to beginning
}

