#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <unistd.h>
#include "jobs.h"

/*
initialize: Function resets the Command structure by setting arg count and background to 0. Also setting argv array to NULL.
            This ensures that no residual data from a previous command interferes with the next one. 

@param command: Pointer to the Command structure.
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

 @param input_size: The number of bytes read from the user input.
 @param max_limit: The maximum number of characters allowed for a command line.
 @param buffer: Pointer to the input buffer containing the user's command.
 
 @return 1 if the input exceeds the maximum limit (too long), 0 otherwise.
*/
int char_limit(ssize_t input_size, int max_limit, char *buffer);

/*
tokenize_command: Function splits user inputs into tokens to make commands.
                  Is done by parsing the user input stored in the buffer and separating 
                  given inputs and storing them as tokens(arguments).
                  Each token is dynamically allocated using the custom mystrdup() function
                  and stored in the argv array of the Command structure.
                  The function also detects the '&' operator and stores it as a separate token,
                  allowing for background execution handling later.

                  Tokenization stops when the end of the buffer is reached, a null terminator is found,
                  or when the maximum number of allowed arguments (MAX_ARGS) is reached.

 @param i: The current index in the input buffer where tokenization should begin.
 @param input: The total number of bytes read into the buffer (upper bound for parsing).
 @param buffer: The character array containing the raw user input.
 @param command: Pointer to the Command structure where parsed arguments are stored.
 
 @return 0 on successful tokenization,
          -1 if memory allocation for any token fails.
 */
int tokenize_command(int i, int input, char *buffer, struct Command *command);

/*
handle_background: Function handles background requests, specifically checks for the ampersand (&).
                   As this is indicative of whether wants to run a program in the background. If 
                   the '&' is found in the line of input, background is set to 1 and '&' is removed.

                   Otherwise, if no '&' is found background is set to 0 and the argv array is explicitly set
                   to NULL-terminate, which makes the command run in foreground by default.

@param command: Pointer to the Command structure.
*/
//void handle_background(struct Command *command);
char *resolve_path(const char *cmd);

int get_command(struct Command *command);
int run_command(struct Command *command);

#endif