#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <unistd.h>
#include "jobs.h"

/*
start_char: Function scans through the input buffer up to the number of bytes read, 
            to find the first non white-space and return its index. 
            Primarily used for command parsing so no white-spaces are included when tokenizing input.

@param buffer: Pointer to the character array containing user input.
@param bytes_read: Number of characters read into the input_buffer.

@return current index of the first character within the buffer.
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
tokenize_command: Function splits user inputs into tokens (words / operators) to make commands.
                  Initially the function loops through the input buffer, skipping whitespace and
                  identifying whether the next character sequence is a normal word or
                  an operator. Operators are handled by tokenize_operator(), and words
                  are handled by tokenize_word(). Each token is dynamically allocated
                  using mystrdup() and stored in the argv array, while argc is incremented.

                  Tokenization stops once the buffer ends, a null terminator is found,
                  or the maximum argument limit (MAX_ARGS) is reached.

 @param index: The current index of the first char which is non-whitespace.
 @param bytes_read: The total number of bytes read into the buffer.
 @param buffer: The character array containing the user's command input.
 @param command: Pointer to the Command structure where tokens are stored.
 
 @return 0 on successful tokenization,
          -1 if memory allocation for any token fails or MAX_ARGS exceeded.
 */
int tokenize_command(int i, int input, char *buffer, struct Command *command);

/*
tokenize_operator: Function handles special shell operators such as '&', '<', '>', and '|'.
                   These symbols are each treated as individual tokens and stored separately
                   in the Command structure. This distinction allows later parsing functions
                   (e.g., for redirection or background jobs) to easily detect and process
                   operators.

                   Each operator is duplicated using mystrdup() and stored in the argv array.
                   The argument count (argc) is then incremented, and the parsing index is
                   advanced by one position to continue reading the next character.

 @param index: The current index of the first char which is non-whitespace.
 @param buffer: The character array containing the user's command input.
 @param command: Pointer to the Command structure where tokens are stored.

 @return The updated index position after processing the operator,
         -1 if memory allocation fails or the argument limit (MAX_ARGS) is reached.
*/
int tokenize_operator(int i, char *buffer, struct Command *command);

/*
tokenize_word: Function handles the extraction of normal words (commands or arguments)
               from the user input. It reads consecutive non-whitespace and non-operator
               characters to form a complete token, which is then stored in the Command
               structure’s argv array.

               The token is dynamically allocated using mystrdup(), ensuring each
               argument is stored safely and independently. The function stops reading
               when it encounters a space, tab, operator symbol ('&', '<', '>', '|'),
               or the end of the input buffer.

 @param index: The current index of the first char which is non-whitespace.
 @param bytes_read: The total number of characters read into the buffer.
 @param buffer: The character array containing the user's command input.
 @param command: Pointer to the Command structure where parsed tokens are stored.

 @return The updated index position after processing the current word,
         -1 if memory allocation fails or the argument limit (MAX_ARGS) is exceeded.
*/
int tokenize_word(int i, int input, char *buffer, struct Command *command);

char *resolve_path(const char *cmd);
void redirect_input(int in_fd);
void redirect_output(int out_fd);

int get_command(struct Command *command);
pid_t run_command(struct Command *command, struct FD *fd_set);

#endif