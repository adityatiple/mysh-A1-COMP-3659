#ifndef JOBS_H
#define JOBS_H
#include <sys/types.h>

#define MAX_ARGS 16     
#define MAX_CH 256     
#define MAX_PIPELINE_LEN 2

/*
struct Command:
    Represents a single command and its arguments in the shell.

    Each Command holds an argument vector (argv[]) and a count (argc),
    similar to how execve() expects arguments. The argv array stores
    individual tokens parsed from the user’s input line.

 Fields:
    argv[] : Array of argument strings (tokens).
    argc   : Number of valid arguments currently stored in argv[].
*/
struct Command
{
  char *argv[MAX_ARGS+1]; // argument vector - array of argument strings
  int argc;               // argument count - number of arguments   
};

/*
struct Job:
    Represents a complete job entered by the user — including one or two 
    pipeline stages, redirection paths, and background execution state.

    Each job contains up to two Command structures (stage 0 and stage 1),
    depending on whether a pipe ('|') is present. The Job also tracks file
    redirection paths and whether the command should run in the background.

 Fields:
    pipeline[]   : Array of Command structures (stage 0 and optional stage 1).
    num_stages   : Number of active stages (1 = normal command, 2 = pipeline).
    infile_path  : Input redirection path (after '<'), or NULL if none.
    outfile_path : Output redirection path (after '>'), or NULL if none.
    background   : 0 = foreground process, 1 = background process ('&' used).
*/

struct Job {
    struct Command pipeline[MAX_PIPELINE_LEN];
    int num_stages;            // to determine if | or not 
    char *infile_path;         // NULL if not specified
    char *outfile_path;        // NULL if not specified    
    int background;            // 0 = foreground, 1 = background
};

/*
struct FD:
    Groups all file descriptors used when executing a Job.

    This structure centralizes descriptor management for both stages
    (when a pipeline exists) and for input/output redirection.

    It helps ensure that proper ends of pipes and redirection files are
    closed at the correct times to avoid descriptor leaks.

 Fields:
    in_fd        : File descriptor for input redirection ('<'), or -1 if none.
    out_fd       : File descriptor for output redirection ('>'), or -1 if none.
    pipefd[2]    : Pipe file descriptors; [0] = read end, [1] = write end.
    pipe_exists  : Boolean flag (1 if pipe is created, 0 otherwise).

    stage0_in    : Standard input FD for stage 0.
    stage0_out   : Standard output FD for stage 0.
    stage1_in    : Standard input FD for stage 1 (if pipeline used).
    stage1_out   : Standard output FD for stage 1.
*/
struct FD {
    int in_fd;
    int out_fd;
    int pipefd[2];   // [0] = read, [1] = write 
    int pipe_exists;

    // per-stage stdio */
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

@param job: Pointer to the Job structure containing the parsed commands,
            redirection paths, and background flags for the current job.

 @return 0  → Success (valid or no pipe found)
        -1  → Error (syntax error or multiple pipes detected)
*/
int parse_pipeline(struct Job *job);

/*
parse_input_redirection: Function scans the first pipeline stage for the '<' operator,
                         validates its usage, and records the input filename in Job.
                         It then removes the operator and its filename from argv[] so the
                         execve()-ready argv contains only the actual arguments.

@param job: Pointer to the Job structure containing the parsed commands,
            redirection paths, and background flags for the current job.

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

@param job: Pointer to the Job structure containing the parsed commands,
            redirection paths, and background flags for the current job.

 @return 0  → Success (including when no '>' is present)
        -1  → Error (missing filename or multiple output redirections detected)
*/
int parse_output_redirection(struct Job *job);

/*
setup_redirection: Function opens the input and output files specified in the
                   Job structure for redirection and stores their file
                   descriptors in the FD structure.
                  
                   - If job->infile_path is set, opens the file in read-only mode.
                   - If job->outfile_path is set, opens (or creates) the file in
                     write-only mode, truncating it if it already exists.  

@param job:    Pointer to the Job structure containing the parsed commands,
               redirection paths, and background flags for the current job.
@param fd_set: Pointer to the FD structure containing all input/output and
               pipeline-related file descriptors.

 @return 
         0 → Success (all redirections opened and ready).
        -1 → Failure (error opening input or output file).
*/
int setup_redirection(struct Job *job, struct FD *fd_set);

/*
launch_stage0: Function launches the first command (stage 0) of a job pipeline.
               It sets up appropriate input/output file descriptors depending 
               on redirection or pipeline usage, then forks and executes the 
               command via run_command().

               For single-stage jobs, output may be redirected to a file (">").
               For two-stage pipelines, the write end of the pipe is assigned
               to stage 0's stdout so it can feed into stage 1's stdin.

               The parent process closes any file descriptors it no longer needs
               (such as input files or pipe write ends) to prevent resource leaks
               and ensure proper EOF signaling in pipelines.

@param job:    Pointer to the Job structure containing the parsed commands,
               redirection paths, and background flags for the current job.
@param fd_set: Pointer to the FD structure containing all input/output and
               pipeline-related file descriptors.

 @return On success, returns the PID of the stage 0 child process.
         On failure, returns -1 if process creation or setup fails.
*/
pid_t launch_stage0(struct Job *job, struct FD *fd_set);


/*
launch_stage1: Function launches the second command (stage 1) in a two-stage
               pipeline. It configures stage 1's standard input to read from
               the read end of the pipe created earlier by stage 0, and sets
               up output redirection if specified (e.g., '> filename').

               Once the process is forked and exec'd via run_command(), the
               parent closes its copies of the pipe read end and any output
               file descriptors to prevent descriptor leaks and ensure proper
               EOF behavior in the pipeline.

@param job:    Pointer to the Job structure containing the parsed commands,
               redirection paths, and background flags for the current job.
@param fd_set: Pointer to the FD structure containing all input/output and
               pipeline-related file descriptors.

 @return On success, returns the PID of the stage 1 child process.
         On failure, returns -1 if process creation or setup fails.
*/
pid_t launch_stage1(struct Job *job, struct FD *fd_set);

/*
close_fd: Function safely closes any active file descriptors used for input or
          output redirection within the FD structure. It prevents file descriptor
          leaks by ensuring that only valid, open descriptors (non-negative) are
          closed.

@param fd_set: Pointer to the FD structure containing all input/output and
               pipeline-related file descriptors.
*/
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

/*
wait_for_foreground: Function waits for the completion of one or more foreground
                     child processes (pipeline stages) using waitpid(). It ensures
                     that the shell blocks until all commands in a foreground job
                     have terminated before re-prompting the user.

                     For single-stage jobs, only the first process (p0) is waited on.
                     For two-stage pipelines, both processes (p0 and p1) are reaped
                     in sequence. Any errors during waitpid() calls are reported
                     to stderr but do not terminate the shell.

 @param p0: PID of the first child process (stage 0).
 @param p1: PID of the second child process (stage 1), valid only if a pipe exists.
 @param pipe_exists: Flag indicating whether the job contains a two-stage pipeline
                     (1 = yes, 0 = no).

 @return Returns the exit status of the last waited-on child process.
*/
int wait_for_foreground(pid_t p0, pid_t p1, int pipe_exists);

//----------------------------------------------------------------------------------------------------------------------------

/*
get_job:  Function builds and prepares a complete Job structure from user input.
          It reads a command line, tokenizes it into a Command structure, and
          then applies further parsing to handle operators such as pipes ('|'),
          redirections ('<' and '>'), and background execution ('&').

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

/*
run_job: Function orchestrates execution of a complete Job (one or two stages).
         It initializes file-descriptor state, applies requested I/O redirection,
         optionally creates a pipe for two-stage pipelines, launches stage 0 and
         (if present) stage 1, and then either returns immediately (background)
         or waits for the foreground children to finish.

         On any failure during setup or launch, open FDs (including pipe ends)
         are closed by helpers (e.g., close_fd / launch_* error paths) to avoid
         resource leaks, and the function returns -1.

@param job: Pointer to the Job structure containing the parsed commands,
            redirection paths, and background flags for the current job.

 @return 0 on success (including background submission); 
         -1 on error during redirection setup, pipe creation, or process launch.
*/
int run_job(struct Job *job);  

#endif
