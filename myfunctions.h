#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <unistd.h>
#include "jobs.h"

void initialize(struct Command *command);
int start_char(char *input, int bytes_read);
int char_limit(ssize_t input_size, int max_limit, char *input_buffer);
int tokenize_command(int i, int input, char *buffer, struct Command *command);
void handle_background(struct Command *command);
char *resolve_path(const char *cmd);

int get_command(struct Command *command);
int run_command(struct Command *command);

#endif