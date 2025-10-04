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
static char *arena_ndup(const char *s, unsigned int n) {
    char *p = alloc(n + 1);
    if (!p) return NULL;
    for (unsigned int i = 0; i < n; i++) {
        p[i] = s[i];
    }
    p[n] = '\0';
    return p;
}

int get_command(struct Command *command) {
    char input_buffer[MAX_CH + 1];

    /* reset per-command arena */
    free_all();

    /* init */
    command->argc = 0;
    command->background = 0;
    for (int i = 0; i < MAX_ARGS + 1; i++) {
        command->argv[i] = NULL;
    }

    /* prompt */
    write(1, "mysh $ ", 7);

    /* read (cap at MAX_CH) */
    ssize_t nread = read(0, input_buffer, MAX_CH);
    if (nread <= 0) return -1;                 // EOF or error
    input_buffer[nread] = '\0';

    /* too long? */
    if (char_limit(nread, MAX_CH, input_buffer)) return 0;

    /* strip trailing newline */
    if (nread > 0 && input_buffer[nread - 1] == '\n') {
        input_buffer[nread - 1] = '\0';
        nread--;
    }

    /* skip leading ws */
    int i = start_char(input_buffer, (int)nread);
    if (i >= nread) return 0;                  // only whitespace

    /* exit? */
    if (mystrcmp(input_buffer + i, "exit") == 0) return 1;





    /* tokenize: spaces/tabs; '&' is its own token */
    while (i < nread && command->argc < MAX_ARGS) {
        /* skip ws */
        while (i < nread && (input_buffer[i] == ' ' || input_buffer[i] == '\t')) i++;
        if (i >= nread || input_buffer[i] == '\0') break;

        if (input_buffer[i] == '&') {
            char *amp = arena_ndup("&", 1);
            if (!amp) { write(2, "alloc failed\n", 13); return -1; }
            command->argv[command->argc++] = amp;
            i++;
            continue;
        }

        int start = i;
        while (i < nread &&
               input_buffer[i] != ' ' && input_buffer[i] != '\t' &&
               input_buffer[i] != '&' && input_buffer[i] != '\0') {
            i++;
        }
        unsigned int len = (unsigned int)(i - start);
        if (len > 0) {
            char *tok = arena_ndup(input_buffer + start, len);
            if (!tok) { write(2, "alloc failed\n", 13); return -1; }
            command->argv[command->argc++] = tok;
        }
        /* loop continues; if we stopped on '&', it will be handled next */
    }




    command->argv[command->argc] = NULL;   // important!
    return 0;
}

int run_command(struct Command *command) {
    char * const newenvp[] = {NULL};
    int status = 0;
    pid_t pid;

    if (!command || command->argc == 0 || !command->argv[0])
        return 0;

    

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
