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
 @param command: Pointer to the Command structure where tokens are stored and counted.
 
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
 @param command: Pointer to the Command structure where the total count and the actual token values are stored.

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
 @param command: Pointer to the Command structure where tokens are stored and counted.

 @return The updated index position after processing the current word,
         -1 if memory allocation fails or the argument limit (MAX_ARGS) is exceeded.
*/
int tokenize_word(int i, int input, char *buffer, struct Command *command);

/*
resolve_path: Function Resolves the full path of a command to ensure it can be executed by execve(), which requires an absolute or relative pathname.
              the command already includes a '/' (indicating it’s already a path), the function returns it unchanged.
              Otherwise, it automatically prepends the directory "/usr/bin/" to the command name to form a complete path.

@param command: Pointer to the Command structure where tokens are stored and counted.

@return The resolved path string:
         - Returns the same argv[0] pointer if it already contains '/'.
         - Returns a newly allocated string containing "/usr/bin/" + argv[0].
         - Returns NULL if memory allocation fails.

*/
char *resolve_path(const char *command);

/*
redirect_input: Function redirects the standard input (stdin) of the current process
                to the file descriptor provided by in_fd. This allows commands that
                use input redirection ("<") to read from a file instead of the terminal.

                If the given file descriptor is valid (>= 0), dup2() replaces stdin (fd 0)
                with in_fd. After duplication, the original descriptor is closed since
                it is no longer needed.

                On failure, an error message is printed to stderr and the process
                terminates immediately using _exit(1).

 @param in_fd: The file descriptor of the input file to be redirected to stdin.
               If negative, no redirection occurs.

 @return None (void function). Process exits on failure.
*/
void redirect_input(int in_fd);

/*
redirect_output: Function redirects the standard output (stdout) of the current process
                 to the file descriptor provided by out_fd. This enables output
                 redirection (">") so that a command writes its results to a file
                 instead of the terminal.

                 If the given file descriptor is valid (>= 0), dup2() replaces stdout
                 (fd 1) with out_fd. After duplication, the original descriptor is
                 closed since it is no longer needed.

                 On failure, an error message is printed to stderr and the process
                 terminates immediately using _exit(1).

 @param out_fd: The file descriptor of the output file to be redirected to stdout.
                If negative, no redirection occurs.

 @return None (void function). Process exits on failure.
*/
void redirect_output(int out_fd);

/*
get_command: Function reads a full command line from standard input, tokenizes it,
              and populates the provided Command structure with arguments and operators.
              It also handles shell-specific features like whitespace skipping, detecting
              the "exit" command, and checking if the given input is within the assigned limit.

              The user is first shown the prompt "mysh $ ". Input is read using read(),
              stored into a temporary buffer, and cleaned of any trailing newline.
              The command is then tokenized using tokenize_command(), which breaks it
              into separate words and operators.

              If the command is blank or exceeds the maximum input length, the function
              returns 0 and the shell re-prompts the user.

 @param command: Pointer to the Command structure where tokens are stored and counted.

 @return 
         1 → If the user entered "exit" (shell should terminate).
         0 → For blank lines, invalid input, or tokenization errors (reprompt user).
*/
int get_command(struct Command *command);

/*
run_command: Function executes a single Command structure in a child process.
              It forks the current process, sets up input/output redirections
              using the provided FD structure, and finally calls execve() to
              execute the resolved command.

              The parent process receives the child's PID and continues execution,
              while the child process replaces its image with the target program.

              This function is the core of process creation for both single and
              pipelined jobs, handling the execution stage for each command in
              the Job’s pipeline.

 @param command: Pointer to the Command structure where tokens are stored and counted.
 @param fd_set:  Pointer to the FD structure where file descriptors for I/O are stored.

 @return The PID of the newly created child process if fork succeeds.
         -1 if fork fails.
         0  if command or argv[0] is empty (no execution).
*/
pid_t run_command(struct Command *command, struct FD *fd_set);

#endif