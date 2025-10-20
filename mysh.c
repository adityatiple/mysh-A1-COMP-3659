#include "mystring.h"
#include "myfunctions.h"
#include "jobs.h"
#include "myheap.h"

/*
main: Entry point for the mysh shell program.
      This function continuously prompts the user, reads a command,
      parses it into a Job structure, executes it (foreground or background),
      and resets the custom heap allocator before reading the next command.

      The loop terminates only when the user explicitly types "exit",
      which is detected and returned by get_job().

 @param argc: Number of command-line arguments passed to mysh.
 @param argv: Array of command-line argument strings.

 @return 0 upon successful shell termination.
*/
int main(int argc, char *argv[]) {    
    
    struct Job job;                        // Holds parsed command data (pipeline, redirection, background flag)
    int job_status;                        // Status code returned by get_job()

    while(1) {
        job_status = get_job(&job);        // Reads and tokenizes the user input into a Job structure
        if (job_status == 1) {             // "exit" was typed
            break;
        }
        if (job_status == 0) {             // blank line / parsing error
            continue;
        }
        run_job(&job);                     // runs foreground or backgrounds
        free_all();                        // resets heap per-command
    }
    return 0;    
}
