#include "mystring.h"
#include "myfunctions.h"
#include "jobs.h"
#include "myheap.h"
#include <stdio.h> //remove later


int main(int argc, char *argv[], char *envp[])
{
  struct Command command;
  int exitShell = 0;
  /* TO DO: prompt for and read command line */
  
  while (1)
    {
      int result = get_command(&command); 
      if (result == 1) break; // exit the shell
      

 /*char *p1 = alloc(20);
    if (p1 == NULL) {
        printf("alloc failed for p1\n");
        return 1;
    }
    mystrcpy(p1, "hello");
    printf("p1 = %s\n", p1);

    // Allocate another block
    char *p2 = alloc(30);
    if (p2 == NULL) {
        printf("alloc failed for p2\n");
        return 1;
    }
    mystrcpy(p2, "world!");
    printf("p2 = %s\n", p2);

    // Show both still exist
    printf("Together: %s %s\n", p1, p2);

    // Reset the heap
    free_all();
    printf("Heap reset done\n");

    // Allocate again after free_all()
    char *p3 = alloc(15);
    if (p3 == NULL) {
        printf("alloc failed for p3\n");
        return 1;
    }
    mystrcpy(p3, "new");
    printf("p3 = %s\n", p3); */

      
      
      /* TO DO: process command line */




      /* TO DO: prompt for and read command line */
    }
  
  return 0;
}
