#ifndef MY_HEAP_H
#define MY_HEAP_H

#include "jobs.h"

/*
alloc:  Function allocates a continuous block of memory from the custom heap.
        This memory is used for all shell allocations (e.g., command tokens,
        duplicated strings, resolved paths) instead of malloc().

        The heap grows linearly — each call to alloc() moves the heap pointer
        forward by the requested size. Memory is never freed individually;
        instead, all allocations are cleared together using free_all().

 @param size: The number of bytes to allocate from the heap.

 @return Pointer to the start of the allocated memory block.
         Returns NULL if there is not enough space remaining in the heap.
*/
char *alloc(int size);

/*
free_all:  Function resets the heap pointer back to the beginning of the
           memory region, effectively releasing all memory allocated since
           the shell started.

           This approach allows for simple and fast memory management where
           all per-command allocations (strings, tokens, etc.) are cleared
           at once after each command execution.

 @return None (void function).
*/
void free_all();

#endif
