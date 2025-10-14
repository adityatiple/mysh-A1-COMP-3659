#include "mystring.h"
#include "myfunctions.h"
#include "jobs.h"
#include "myheap.h"


int main(int argc, char *argv[], char *envp[]) {    
    
    struct Job job;
    int job_status;

    while(1) {
        job_status = get_job(&job);
        if (job_status == 1) {              // "exit" was typed
            break;
        }
        if (job_status == 0) {           // blank line / parsing error
            continue;
        }
        run_job(&job);             // runs foreground or backgrounds
        free_all();                        // resets heap per-command
    }

    return 0;  
  
}
