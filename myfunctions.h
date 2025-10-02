#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <unistd.h>
#include "jobs.h"

int start_char(char *input, int bytes_read);
int exit_command(char* input_buffer, int start);
int char_limit(ssize_t input_size, int max_limit, char *input_buffer);

int get_command(struct Command *command);
int run_command(struct Command *command);

#endif