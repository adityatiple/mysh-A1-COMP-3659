#include "mystring.h"
#include "myfunctions.h"
#include "jobs.h"
#include <stdio.h>


int main(int argc, char *argv[], char *envp[])
{
  struct Command command;
  int exitShell = 0;
  /* TO DO: prompt for and read command line */
  
  while (1)
    {
      int result = get_command(&command); 

      if (result == 1) break; // exit the shell
      
      /* TO DO: process command line */
      /* TO DO: prompt for and read command line */
    }
  
  return 0;
}
