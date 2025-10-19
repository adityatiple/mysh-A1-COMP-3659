#ifndef JOBS_H
#define JOBS_H
#include <sys/types.h>

#define MAX_ARGS 16     
#define MAX_CH 256     
#define MAX_PIPELINE_LEN 2

struct Command
{
  char *argv[MAX_ARGS+1]; // argument vector - array of argument strings
  int argc;      // argument count - number of arguments   
};

struct Job {
    struct Command pipeline[MAX_PIPELINE_LEN];
    int num_stages;   // 1 for now
    char *infile_path;         // NULL if not specified
    char *outfile_path;        // NULL if not specified    
    int background;            // 0 = foreground, 1 = background
};

struct FD {
    int in_fd;
    int out_fd;
    int pipefd[2];   /* [0] = read, [1] = write */
    int pipe_exists;

    /* per-stage stdio (parent-side view) */
    int stage0_in, stage0_out;
    int stage1_in, stage1_out;
};

/********************************************************* HELPER FUNCTIONS **********************************************************/

/*
initialize_command: Function resets and prepares a Command structure for use.
                    It clears all argument entries, resets argc to zero, and
                    ensures argv[] is NULL-terminated. This prevents leftover
                    data from previous commands from interfering with current 
                    parsing or execution.

 @param command:    Pointer to the Command structure where tokens are stored and counted.

 @return None (void function).
*/
void initialize_command(struct Command *command);

/*
initialize_job:  Function resets and prepares a Job structure before parsing a new
                 user command. It clears all pipeline stages, resets input/output
                 paths, and disables background execution.

                 Each stage of the pipeline[] array is initialized individually
                 using initialize_command().

@param job: Pointer to the Job structure containing the parsed commands,
            redirection paths, and background flags for the current job.

 @return None (void function).
*/
void initialize_job(struct Job *job);

/*
initialize_fd: Function resets all file descriptor fields in an FD structure
               to default values (-1) before a new job is launched. This ensures
               that no stale or inherited descriptors interfere with the current
               command’s redirection or piping setup.

 @param fd_set: Pointer to the FD structure containing all input/output and
                pipeline-related file descriptors.

 @return None (void function).
*/
void initialize_fd(struct Job *job, struct FD *fd_set);

/*
find_token: Function checks to find, if the given string matches any tokens within the argv array.
            This is done by the use of mystrcmp(), if string is found in the array return its index position.


 @param command: Pointer to the Command structure where tokens are stored.
 @param token:   User input stored in the current command strcut within local memory. 

 @return the index position of the token in the argv array which matches the given string. 
*/
static int find_token(struct Command *command, const char *token);

/*
remove_tokens: Function removes a consecutive slice of tokens from a Command's argv[]
               and compacts the array so it is ready for execve(). This is used after
               parsing shell syntax (e.g., "< file", "> file") so that only the true
               program and its arguments remain in argv[].

 @param command: Pointer to the Command structure where tokens are stored and counted.
 @param start:   The starting index of the first token to remove.
 @param count:   The number of consecutive tokens to remove from argv[].

 @return None (void).
*/
static void remove_tokens(struct Command *cmd, int pos, int count);

/*
parse_pipeline: Function detects and handles the pipe operator ('|') within the
                first command stage of a Job structure. It divides the tokens
                into two separate Command structures (stage 0 and stage 1)
                when a single '|' is present. Command 0 contains all tokens 
                before '|' and Command 1 the ones after. Lastly checks if 
                commands contain more than one '|', if so write error msg 
                and exit with -1.

 @param job: Pointer to the Job structure that contains one or more commands
             in its pipeline[] array.

 @return 0  → Success (valid or no pipe found)
        -1  → Error (syntax error or multiple pipes detected)
*/
int parse_pipeline(struct Job *job);

/*
parse_input_redirection: Function scans the first pipeline stage for the '<' operator,
                         validates its usage, and records the input filename in Job.
                         It then removes the operator and its filename from argv[] so the
                         execve()-ready argv contains only the actual arguments.

 @param job: Pointer to the Job structure whose first stage (pipeline[0]) may
             contain an input redirection operator and filename.

 @return 0 on success (including when no "<" is present),
         -1 on errors (missing filename after '<', or multiple '<').
*/
int parse_input_redirection(struct Job *job);

/*
parse_output_redirection: Function scans the final command stage in a Job structure
                          for the '>' operator, validates its usage, and records
                          the output filename in Job. It then removes
                          the operator and its filename from the argv[] array so that
                          execve() receives only the actual arguments.

                          This function supports both single-stage and two-stage
                          (pipelined) jobs. For a pipeline, only the last stage
                          (pipeline[1]) is checked for output redirection.

 @param job: Pointer to the Job structure whose pipeline will be inspected for
             an output redirection operator and corresponding filename.

 @return 0  → Success (including when no '>' is present)
        -1  → Error (missing filename or multiple output redirections detected)
*/
int parse_output_redirection(struct Job *job);

int setup_redirection(struct Job *job, struct FD *fd_set);

pid_t launch_stage0(struct Job *job, struct FD *fd_set);
pid_t launch_stage1(struct Job *job, struct FD *fd_set);
void close_fd(struct FD *fd_set);

/*
handle_background: Function detects the background operator ('&') at the end of
                   the final command stage in a Job structure. When found, it
                   sets the job to run in the background and removes '&' from
                   the command’s argv[] list so execve() only receives valid
                   arguments.

@param job: Pointer to the Job structure containing the parsed commands,
            redirection paths, and background flags for the current job.

 @return None (void function).
*/
void handle_background(struct Job *job);
int wait_for_foreground(pid_t p0, pid_t p1, int pipe_exists);



//----------------------------------------------------------------------------------------------------------------------------

/*
get_job:  Function builds and prepares a complete Job structure from user input.
          It reads a command line, tokenizes it into a Command structure, and
          then applies further parsing to handle operators such as pipes ('|'),
          redirections ('<' and '>'), and background execution ('&').

          The function coordinates all command-level parsing functions
          (parse_pipeline, parse_input_redirection, parse_output_redirection,
          and handle_background) to fully initialize the Job before execution.

          If the user enters "exit", the function signals the shell to terminate.
          If the input is blank or invalid, it returns 0 and the shell re-prompts.

@param job: Pointer to the Job structure containing the parsed commands,
            redirection paths, and background flags for the current job.

 @return 
         1 → If the user entered "exit" (shell should terminate).
         0 → For blank input, invalid commands, or parsing errors (reprompt user).
         2 → Successful parsing of a complete and valid job.
*/
int get_job(struct Job *job);  
int run_job(struct Job *job);  

#endif
