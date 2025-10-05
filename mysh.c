#include "mystring.h"
#include "myfunctions.h"
#include "jobs.h"
#include "myheap.h"
#include <stdio.h> //remove later


static void debug_print_command(const struct Command *cmd) {
    printf("argc=%u, background=%d\n", cmd->argc, cmd->background);
    for (unsigned i = 0; i < cmd->argc; i++) {
        printf("  argv[%u] = \"%s\"\n", i, cmd->argv[i]);
    }
    if (cmd->argc == 0) puts("  (no tokens)");
}


int main(int argc, char *argv[], char *envp[])
{    struct Command cmd;

    // Test 1: /bin/ls -l
    cmd.argv[0] = "/bin/ls";
    cmd.argv[1] = "-l";
    cmd.argv[2] = NULL;
    cmd.argc = 2;
    cmd.background = 0;
    printf("Running: /bin/ls -l\n");
    run_command(&cmd);

    // Test 2: /bin/echo hello world
    cmd.argv[0] = "/bin/echo";
    cmd.argv[1] = "hello";
    cmd.argv[2] = "world";
    cmd.argv[3] = NULL;
    cmd.argc = 3;
    cmd.background = 0;
    printf("Running: /bin/echo hello world\n");
    run_command(&cmd);

    // Test 3: /bin/sleep 2 &
    cmd.argv[0] = "/bin/sleep";
    cmd.argv[1] = "2";
    cmd.argv[2] = NULL;
    cmd.argc = 2;
    cmd.background = 1;
    printf("Running in background: /bin/sleep 2\n");
    run_command(&cmd);

    // Test 4: bad command (should print "execve failed")
    cmd.argv[0] = "/bin/does-not-exist";
    cmd.argv[1] = NULL;
    cmd.argc = 1;
    cmd.background = 0;
    printf("Running: /bin/does-not-exist\n");
    run_command(&cmd);

    //---------------------------------------------GET CMD STUFF-------------------------------------------

   /* for (;;) {
        int rc = get_command(&cmd);
        if (rc == 1) {               // user typed "exit"
            break;
        }
        if (rc < 0) {                // read or parse error
            perror("get_command");
            continue;
        }
        if (cmd.argc == 0) {         // blank/whitespace line
            continue;
        }

        // Optional: sanity check what tokenizer produced
        debug_print_command(&cmd);

        // Execute
        if (run_command(&cmd) < 0) {
            perror("run_command");
        }*/

 //-------------------------------------------HEAP STUFFF---------------------------------------------------
   
  //struct Command command;
  //int exitShell = 0;
  /* TO DO: prompt for and read command line */
  
  //while (1)
    //{
      //int result = get_command(&command); 
      //if (result == 1) break; // exit the shell
      

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
    
  
//  return 0;
}
