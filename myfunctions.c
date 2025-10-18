#include "myfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mystring.h"
#include "myheap.h"
#include "jobs.h"

/******************************************************** HELPER FUNCTIONS ************************************************************/

int start_char(char *buffer, int bytes_read) {
    for (int i = 0; i < bytes_read; i++) {
        if (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\n') {
            return i;                                                       // return first non-whitespace character
        }
    }
    return bytes_read;                                                      // all characters are whitespace
}

int char_limit(ssize_t input_size, int max_limit, char *buffer) {
    int too_long = 0;
    if (input_size > max_limit) {
        too_long = 1;
    } else if (input_size == max_limit && buffer[max_limit - 1] != '\n') {
        too_long = 1;
    }    
    if (too_long) {    
        char discard;                                                       // Clear input buffer.
        while (read(0, &discard, 1) > 0 && discard != '\n');                // read returns > 0 bytes read            
        return 1;                                                           // too long
    }
    return 0;
}

int tokenize_command(int index, int bytes_read, char *buffer, struct Command *command) {
    while (index < bytes_read && command->argc < MAX_ARGS) {
        /* skip whitespace in between*/
        while (index < bytes_read && (buffer[index] == ' ' || buffer[index] == '\t'))
            index++;

        if (index >= bytes_read || buffer[index] == '\0')
            break;

        /* decide what to do */
        if (buffer[index] == '&' || buffer[index] == '<' || buffer[index] == '>' || buffer[index] == '|') {
            index = tokenize_operator(index, buffer, command);
            if (index < 0) return -1;
            continue;
        } else {
            index = tokenize_word(index, bytes_read, buffer, command);
            if (index < 0) return -1;
        }
    }
    return 0;
}

int tokenize_operator(int index, char *buffer, struct Command *command) {
    if (command->argc >= MAX_ARGS) {
        write(2, "too many arguments\n", 19);
        return -1;
    }
    char *token = mystrdup(buffer + index, buffer + index + 1);
    if (!token) {
        write(2, "alloc failed\n", 13);
        return -1;
    }
    command->argv[command->argc++] = token;
    index++;   // move past this operator
    return index;
}

int tokenize_word(int index, int bytes_read, char *buffer, struct Command *command) {
    int start = index;    
    while (index < bytes_read &&                                    /*finding the end of the word (boundary)*/
           buffer[index] != ' ' && buffer[index] != '\t' &&
           buffer[index] != '&' && buffer[index] != '<' && buffer[index] != '>' &&
           buffer[index] != '|' && buffer[index] != '\0') {
        index++;
    }
    int len = index - start;
    if (len > 0) {
        if (command->argc >= MAX_ARGS) {
            write(2, "too many arguments\n", 19);
            return -1;
        }
        char *token = mystrdup(buffer + start, buffer + start + len);
        if (!token) {
            write(2, "alloc failed\n", 13);
            return -1;
        }
        command->argv[command->argc++] = token;
    }
    return index;
}

char *resolve_path(const char *command) {    
    for (const char *p = command; *p != '\0'; p++) {
        if (*p == '/') {                                // Check if command already contains a '/' → treat as full path
            return (char *)command;                     // already absolute/relative
        }
    }
    const char *prefix = "/usr/bin/";
    int total = mystrlen(prefix) + mystrlen(command) + 1;

    char *path = alloc(total);
    if (!path) return NULL;
    mystrcpy(path, prefix);
    mystrcat(path, command);
    return path;
}

void redirect_input(int in_fd) {
    if (in_fd < 0)
        return; // nothing to redirect

    int alias_fd = dup(in_fd);
    if (alias_fd == -1) {
        write(2, "dup(in) failed\n", 15);
        _exit(1);
    }
    if (dup2(alias_fd, 0) == -1) {
        write(2, "dup2(in) failed\n", 16);
        _exit(1);
    }
    close(in_fd);
    close(alias_fd);
}

void redirect_output(int out_fd) {
    if (out_fd < 0)
        return; // nothing to redirect

    int alias_fd = dup(out_fd);
    if (alias_fd == -1) {
        write(2, "dup(out) failed\n", 16);
        _exit(1);
    }
    if (dup2(alias_fd, 1) == -1) {
        write(2, "dup2(out) failed\n", 17);
        _exit(1);
    }
    close(out_fd);
    close(alias_fd);
}

//-------------------------------------------------------------------------------------------------------------------------------

int get_command(struct Command *command) {    
    char buffer[MAX_CH + 1];                                                // input command-line    

    write(1, "mysh $ ", 7);                                                 /* prompt */
    
    ssize_t bytes_read = read(0, buffer, MAX_CH + 1);                       /* bytes read from buffer */
    if (bytes_read <= 0) return 0;                                          // 0 = EOF, -1 = error
    buffer[bytes_read] = '\0';

    if (char_limit(bytes_read, MAX_CH, buffer)) {   
        write(1, "Error: Input exceeds maximum character limit.\n", 46);    /* too long */
        return 0;                                                           /*repromt*/
    }        
    int index = start_char(buffer, (int)bytes_read);                        /* first non-whitespace index */
    if (index >= bytes_read)                                                // i - index of first non white-space
        return 0;   

    if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {                 /* strip trailing newline */
        buffer[bytes_read - 1] = '\0';
        bytes_read--;
    }
    if (mystrcmp(buffer + index, "exit") == 0) {
        write(1, "Exiting shell...\n", 17);                                 /* exit */
        return 1;
    }
    if (tokenize_command(index, (int)bytes_read, buffer, command) == -1)
        return 0;

    command->argv[command->argc] = NULL;                                    // important!
    return 0;
}

pid_t run_command(struct Command *command, int in_fd, int out_fd) {
    if (!command || command->argc == 0 || !command->argv[0])
        return 0;

    pid_t pid = fork();
    if (pid < 0) {
        write(2, "fork failed\n", 12);
        return -1;
    }
    if (pid == 0) {        
        redirect_input(in_fd);
        redirect_output(out_fd);

        char *path = resolve_path(command->argv[0]);
        if (!path) {
            write(2, "alloc failed\n", 13);
            _exit(1);
        }
        execve(path, command->argv, NULL);
        write(2, "execve failed, please re-enter command\n", 40);
        _exit(1);
    }
    return pid; // parent
}



