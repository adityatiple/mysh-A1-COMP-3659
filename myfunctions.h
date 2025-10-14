#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <unistd.h>
#include "jobs.h"

/*
initialize: Function resets the Command structure by setting arg count and background to 0. Also setting argv array to NULL.
            This ensures that no residual data from a previous command interferes with the next one. 

@param command: Pointer to the Command structure to initialize.
*/
void initialize(struct Command *command);

/*
start_char: Function scans through the input buffer up to the number of bytes read, 
            to find the first non white-space and return its index. 
            Primarily used for command parsing so no white-spaces are included when tokenizing input.

@param buffer: Pointer to the character array containing user input.
@param bytes_read: Number of characters read into the input_buffer.

*/
int start_char(char *buffer, int bytes_read);

/*
char_limit: Function checks wheter the user's input is within the max character limit(256).
            If not(input too long), the remainder of the user's input after max_limit is discarded,
            so the buffer remains cleared for the next incoming reads.

*/
int char_limit(ssize_t input_size, int max_limit, char *buffer);
int tokenize_command(int i, int input, char *buffer, struct Command *command);
void handle_background(struct Command *command);
char *resolve_path(const char *cmd);

int get_command(struct Command *command);
int run_command(struct Command *command);

#endif