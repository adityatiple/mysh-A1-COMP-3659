#include "mystring.h"
#include "myfunctions.h"
#include "jobs.h"
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
      

    // Test 4: bad command (should print "execve failed")
    command.argv[0] = "/bin/does-not-exist";
    command.argv[1] = NULL;
    command.argc = 1;
    command.background = 0;
    printf("Running: /bin/does-not-exist\n");
    run_command(&command);

      
      
      /* TO DO: process command line */




      /* TO DO: prompt for and read command line */
    }
  
  return 0;
}
