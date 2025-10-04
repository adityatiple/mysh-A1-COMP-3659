#include "myfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mystring.h"
#include "myheap.h"
#include <stdio.h>
#include <string.h>
#include "jobs.h"

/*************** ERROR HANDLING FUNCTIONS **************/

/* returns the index of the first non whitespace character - needed for tokenizing*/
int start_char(char *input_buffer, int bytes_read) {
    for (int i = 0; i < bytes_read; i++) {
        if (input_buffer[i] != ' ' && input_buffer[i] != '\t' && input_buffer[i] != '\n') {
            return i; // return first non-whitespace character
        }
    }
    return bytes_read; // all characters are whitespace
}

/*  */
int exit_command(char* input_buffer, int start) {
    // strcmp returns 0 when the strings are identical.
    if (mystrcmp(input_buffer + start, "exit") == 0) {
        write(1, "Exiting shell...\n", 17);
        return 1; // signal to main() to exit
    }
    return 0; // Not the exit command
}

int char_limit(ssize_t input_size, int max_limit, char *input_buffer) {
    if (input_size > max_limit) {
        if (input_buffer[max_limit] != '\n') {
            // Clear input buffer.
            char discard;
            while (read(0, &discard, 1) > 0 && discard != '\n');
            write(1, "Error: Input exceeds maximum character limit.\n", 46);
            return 1; // too long
        }
    }
    return 0;
}

/*
Prompts the user to enter a command line
Reads a command line into a buffer, handling cases such as command lines that are 
too long
Calls free_all to reset the heap
Tokenizes the command line, populating the given command structure 
(and its argv, etc.) in the process
*/
int get_command(struct Command *command) {
    char input_buffer[MAX_CH + 1]; // buffer to hold user input

    /* use free_all() function here */

    // initialize argc and argv
    command->argc = 0;
    for (int i = 0; i < MAX_ARGS + 1; i++) {
        command->argv[i] = NULL;
    }
    
    // prompt user
    write(1, "mysh $ ", 7);

    // read user input
    ssize_t input_size = read(0, input_buffer, MAX_CH + 1);
    input_buffer[input_size] = '\0'; // null terminate the end of string

    // ignore leading whitespace + go to first non white space character
    int start = start_char(input_buffer, input_size);
    if (start >= input_size) {
        return 0; // only whitespace
    }
    
    // error handling of maximum character reached
    if (char_limit(input_size, MAX_CH, input_buffer)) {
            return 0;
    }
    
    // Remove trailing newline if present
    if (input_size > 0 && input_buffer[input_size - 1] == '\n') {
        input_buffer[input_size - 1] = '\0';
        input_size--;
    }

    if (exit_command(input_buffer, start)) {
        return 1; // main() will exit
    };

    write(1, input_buffer + start, input_size - start);
    write(1, "\n", 1);

    command->argv[0] = input_buffer + start;
    command->argc = 1;

    /** NEED TO CALL ALLOC AND FREE IN FUNCTION - currently had to hard code **/

    return input_size - start;
}

/* int run_command(struct Command *command) {

    //char * const newargv[] = {"/bin/ls", "-al", NULL};
    char * const newenvp[] = {NULL};

    // Guard clause: nothing to execute (NULL cmd, zero args, or missing program name).
    if (!command || command->argc == 0 || !command->argv[0])
        return 0;

    // handle trailing '&'
    if (command->argc > 0 && mystrcmp(command->argv[command->argc - 1], "&") == 0) {
        command->background = 1; // setting background to 1 since 
        command->argv[command->argc -1] = NULL; // remove '&'
    }
    //calling fork to invoke child process.
    pid_t pid = fork();

    if (pid < 0) {
        write(2,"fork failed\n",12);
        return -1;
    }

     if (pid == 0) {
        // child: NOTE execve does NOT search PATH
        // requires absolute/relative path in argv[0] (e.g., "/bin/ls")

        //execve(command->argv[0], command->argv, NULL);
        execve(command->argv[0], command->argv, newenvp);
     }

    else {            
        // only reached if execve fails
        write(2, "execve failed\n", 14);
        _exit(1); 
        }

    int status = 0;
    if (pid > 0 && command->background == 1) 
        return 0;
    else {
        waitpid(pid, &status, 0);
    }

    return 0; // TO DO
}*/

int run_command(struct Command *command) {
    char * const newenvp[] = {NULL};
    int status = 0;
    pid_t pid;

    if (!command || command->argc == 0 || !command->argv[0])
        return 0;

    // handle trailing '&'
    if (command->argc > 0 && mystrcmp(command->argv[command->argc - 1], "&") == 0) {
        command->background = 1;
        command->argv[command->argc - 1] = NULL; // remove '&'
        command->argc--;
    } else {
        command->background = 0;
        command->argv[command->argc] = NULL;     // ensure NULL-terminated
    }

    pid = fork();
    if (pid < 0) {
        write(2, "fork failed\n", 12);
        return -1;
    } else if (pid == 0) {
        if (execve(command->argv[0], command->argv, NULL) == -1) { // execve always runs, when failed returns -1
            write(2, "execve failed\n", 14);
            _exit(1); // 127 only child exits here on failure
        }
    }

    if (command->background) {        
        return 0; // don't wait; keep shell alive
    }
    
    if (waitpid(pid, &status, 0) < 0) {
        write(2, "waitpid failed\n", 15);
        return -1;
    }
    return 0;
}
