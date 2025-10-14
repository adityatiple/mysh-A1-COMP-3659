#include "mystring.h"
#include "myfunctions.h"
#include "jobs.h"
#include "myheap.h"


int main(int argc, char *argv[], char *envp[]) {    
    
    struct Command command;
    int command_status;

    while(1) {
        command_status = get_command(&command);
        if (command_status == 1) {              // "exit" was typed
            break;
        }
        if (command.argc == 0) {           // blank line / parsing error
            continue;
        }
        run_command(&command);             // runs foreground or backgrounds
        free_all();                        // resets heap per-command
    }

    return 0;  
  
}
