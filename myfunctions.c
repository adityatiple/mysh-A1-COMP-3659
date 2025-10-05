#include "myfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mystring.h"
#include "myheap.h"
#include <stdio.h>
#include <string.h>
#include "jobs.h"

/******************************************************** HELPER FUNCTIONS ************************************************************/

/* returns the index of the first non whitespace character - needed for tokenizing*/
int start_char(char *input_buffer, int bytes_read) {
    for (int i = 0; i < bytes_read; i++) {
        if (input_buffer[i] != ' ' && input_buffer[i] != '\t' && input_buffer[i] != '\n') {
            return i; // return first non-whitespace character
        }
    }
    return bytes_read; // all characters are whitespace
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

int tokenize_command(int i,int input, char *buffer, struct Command *command) {    
    while (i < input && command->argc < MAX_ARGS) { /* tokenize: spaces/tabs; '&' is its own token */
        /* skip ws */
        while (i < input && (buffer[i] == ' ' || buffer[i] == '\t')) i++;

        if (i >= input || buffer[i] == '\0') {
            break;
        }

        if (buffer[i] == '&') {
            char *ampersand = mystrdup("&", "&" + 1);
            if (!ampersand) { 
                write(2, "alloc failed\n", 13); 
                return -1; 
            }
            command->argv[command->argc++] = ampersand;
            i++;
            continue;
        }

        int start = i;
        while (i < input &&
               buffer[i] != ' ' && buffer[i] != '\t' &&
               buffer[i] != '&' && buffer[i] != '\0') {
            i++;
        }
        int len = (int)(i - start);
        if (len > 0) {
            char *token = mystrdup(buffer + start, buffer + start + len);
            if (!token) { 
                write(2, "alloc failed\n", 13);
                return -1; }
            command->argv[command->argc++] = token;
        }
        /* loop continues; if we stopped on '&', it will be handled next */
    }
    return 0;
}

void handle_background(struct Command *command) { 
        // handle trailing '&'
    if (command->argc > 0 && mystrcmp(command->argv[command->argc - 1], "&") == 0) {
        command->background = 1;
        command->argv[command->argc - 1] = NULL; // remove '&'
        command->argc--;
    } else {
        command->background = 0;
        command->argv[command->argc] = NULL;     // ensure NULL-terminated
    }
}



int get_command(struct Command *command) {
    char buffer[MAX_CH + 1];                                    // input command-line
    free_all();                                                 // resets heap per-command 

    /* Resets and initializes */
    command->argc = 0;
    command->background = 0;
    for (int i = 0; i < MAX_ARGS + 1; i++) {
        command->argv[i] = NULL;
    }

    write(1, "mysh $ ", 7);                                     /* prompt */
    
    ssize_t bytes_read = read(0, buffer, MAX_CH);               /* bytes read from buffer */
    if (bytes_read <= 0) return 0;                              //for EOF and error
    buffer[bytes_read] = '\0';

    if (char_limit(bytes_read, MAX_CH, buffer)) {    
        return 0; /* too long? */
    }    
    
    int index = start_char(buffer, (int)bytes_read);            /* first non-whitespace index */
    if (index >= bytes_read)  {                                 // i - index of first non white-space
        return 0;                                   
    }
    
    if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {     /* strip trailing newline */
        buffer[bytes_read - 1] = '\0';
        bytes_read--;
    }

    if (mystrcmp(buffer + index, "exit") == 0) {
        write(1, "Exiting shell...\n", 17);                     /* exit */
        return 1;
    }

    int num_bytes_read = (int)bytes_read;
    if (tokenize_command(index, num_bytes_read, buffer, command) == -1)
        return 0;

    command->argv[command->argc] = NULL;                        // important!
    return 0;
}

int run_command(struct Command *command) {
    char * const newenvp[] = {NULL};
    int status = 0;
    pid_t pid;

    if (!command || command->argc == 0 || !command->argv[0]) {
        return 0;   
    }

    handle_background(command);

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



