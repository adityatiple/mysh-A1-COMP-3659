#include "myfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mystring.h"
#include "myheap.h"
#include "jobs.h"
#include <fcntl.h>
#include <sys/stat.h>

/****************************************************** HELPER FUNCTIONS ************************************************************/

void initialize_command(struct Command *command) {
    command->argc = 0;
    for (int i = 0; i < MAX_ARGS + 1; i++) {
        command->argv[i] = NULL;
    }
}

void initialize_job(struct Job *job) {
    job->num_stages = 0;
    job->outfile_path = NULL;
    job->infile_path = NULL;
    job->background = 0;

    for (int i = 0; i < MAX_PIPELINE_LEN; i++) {
        initialize_command(&job->pipeline[i]);
    }
}

void initialize_fd(struct Job *job, struct FD *fd_set) {
    fd_set->in_fd  = -1;
    fd_set->out_fd = -1;
    fd_set->pipefd[0] = -1;
    fd_set->pipefd[1] = -1;
        
    if (job->num_stages == 2) 
        fd_set->pipe_exists = 1;
    
    fd_set->stage0_in  = -1;
    fd_set->stage0_out = -1;
    fd_set->stage1_in  = -1;
    fd_set->stage1_out = -1;
}

static int find_token(struct Command *command, const char *token) { // used when token location is unknown in the command argv
    for (int i = 0; i < command->argc; i++) {
        if (mystrcmp(command->argv[i], token) == 0) 
            return i;
    }
    return -1;
}

static void remove_tokens(struct Command *command, int start, int count) {
    if (start < 0 || start >= command->argc) return;                // index out of bounds
    
    for (int i = start; i + count < command->argc; i++) {           
        command->argv[i] = command->argv[i + count];                // replaces current tokens with later ones(later ones stay the same)
    }
    
    for (int i = command->argc - count; i < command->argc; i++) {   // Null out the later tokens to avoid duplication
        command->argv[i] = NULL;                                    
    }
    command->argc = command->argc - count;                          // New args count without start and start + 1 tokens
    }

int setup_redirection(struct Job *job, struct FD *fd_set) {

    if (job->infile_path) {                                 // Handle input redirection
        fd_set->in_fd = open(job->infile_path, O_RDONLY);   // set in path and open for reading
        if (fd_set->in_fd < 0) {
            write(2, "open(<) failed\n", 15);
            return -1;
        }
    }    
    if (job->outfile_path) {                                // Handle output redirection
        fd_set->out_fd = open(job->outfile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd_set->out_fd < 0) {
            if (fd_set->in_fd >= 0) close(fd_set->in_fd);   // close in_file path if it was opened
            write(2, "open(>) failed\n", 15);
            return -1;
        }
    }
    return 0;                                               // success - both files opened and set
}

int parse_pipeline(struct Job *job) {
    struct Command *command0 = &job->pipeline[0];
    int bar_index = find_token(command0, "|");
    if (bar_index < 0) {                                                      // no pipe
        job->num_stages = 1;
        return 0;
    }
    if (bar_index == 0 || bar_index == command0->argc - 1) {                  //first or last token is bar 
        write(2, "syntax error near '|'\n", 22);
        return -1;
    }

    initialize_command(&job->pipeline[1]);                                    // initialize the later part of the pipe (after |)
    struct Command *command1 = &job->pipeline[1];    
    for (int i = bar_index + 1; i < command0->argc; i++) {                    // move tokens after '|' into stage-1 
        command1->argv[command1->argc++] = command0->argv[i];
        command0->argv[i] = NULL;
    }    
    command0->argv[bar_index] = NULL;                                         // terminate stage-0 at '|'
    command0->argc = bar_index;

    job->num_stages = 2;    
    if (find_token(command1, "|") >= 0 || find_token(command0, "|") >= 0) {   // error more than one pipe
        write(2, "only one '|' operator supported\n", 32);
        return -1;
    }
    return 0;
}

int parse_input_redirection(struct Job *job) {
    struct Command *command = &job->pipeline[0];        // get tokens before pipe
    int index = find_token(command, "<");               
    if (index < 0) return 0;

    if (index == command->argc - 1) {
        write(2, "missing filename after '<'\n", 27);    // '<' was the last token
        return -1;
    }
    if (job->infile_path != NULL) {                      // was set to NULL
        write(2, "multiple input redirections\n", 28);
        return -1;
    }
    job->infile_path = command->argv[index + 1];         // set infile to token after '<'
    remove_tokens(command, index, 2);                    // remove '<' and filename not needed anymore
    return 0;
}

int parse_output_redirection(struct Job *job) {
    int last = 0;
    if (job->num_stages == 2)                           // check if pipe exists
        last = 1;    

    struct Command *command = &job->pipeline[last];     // get tokens after pipe
    int index = find_token(command, ">");
    if (index < 0) return 0;

    if (index == command->argc - 1) {
        write(2, "missing filename after '>'\n", 27);   // '>' was the last token
        return -1;
    }
    if (job->outfile_path != NULL) {                    // was set to NULL
        write(2, "multiple output redirections\n", 29);
        return -1;
    }
    job->outfile_path = command->argv[index + 1];       // set outfile to token after '>'
    remove_tokens(command, index, 2);                   // remove '>' and filename not needed anymore
    return 0;
}

pid_t launch_stage0(struct Job *job, struct FD *fd_set) {

    if (fd_set->in_fd >= 0) {
        fd_set->stage0_in = fd_set->in_fd;
    }
    if (fd_set->pipe_exists) {
        fd_set->stage0_out = fd_set->pipefd[1];             // stage 0 output becomes input for stage 1
    } else if (job->outfile_path) {
        fd_set->stage0_out = fd_set->out_fd;                // single stage with '<', set ouput to the opened file
    }

    pid_t p0 = run_command(&job->pipeline[0], fd_set); 
    if (p0 <= 0) {
        close_fd(fd_set);                               
        if (fd_set->pipe_exists) {
            if (fd_set->pipefd[0] >= 0) close(fd_set->pipefd[0]);
            if (fd_set->pipefd[1] >= 0) close(fd_set->pipefd[1]);
        }
        return -1;
    }
    if (fd_set->stage0_in >= 0) close(fd_set->stage0_in);   // parent no longer needs it
    if (fd_set->pipefd[1] >= 0) close(fd_set->pipefd[1]);   // close write end of parent 

    return p0;                                              // return the pid (stage 0)
}

pid_t launch_stage1(struct Job *job, struct FD *fd_set) {

    fd_set->stage1_in = fd_set->pipefd[0];                  // input coming from stage 0 's output
    if (job->outfile_path) {
        fd_set->stage1_out = fd_set->out_fd;                // '>' on stage 1 if present
    }

    pid_t p1 = run_command(&job->pipeline[1], fd_set);  

    if (fd_set->stage1_in >= 0) close(fd_set->stage1_in);   // Parent no longer needs to keep the pipe's read end open
    if (fd_set->out_fd >= 0) close(fd_set->out_fd);         // close out_fd to avoid leaks 

    return p1;                                              // return the pid (stage 1)
}

void close_fd(struct FD *fd_set) {
    if (fd_set->in_fd  >= 0) close(fd_set->in_fd);          // close in_fd if open
    if (fd_set->out_fd >= 0) close(fd_set->out_fd);         // close out_fd if open
}

void handle_background(struct Job *job) {               // modifies job->background and strips '&'
    int last = 0;
    if (job->num_stages == 2) 
        last = 1;

    struct Command *command = &job->pipeline[last];
    if (mystrcmp(command->argv[command->argc - 1], "&") == 0) {
        command->argv[command->argc - 1] = NULL;        // Remove "&" from array and set to NULL
        command->argc--;
        job->background = 1;                            // set background to 1
    }
    command->argv[command->argc] = NULL;                // saftery gaurd ensuring array always ends with NULL
}

int wait_for_foreground(pid_t p0, pid_t p1, int pipe_exists) {
    int status = 0;    
    if (p0 > 0) {                                          // Wait for stage 0 
        if (waitpid(p0, &status, 0) < 0) {
            write(2, "waitpid stage0 failed\n", 22);
        }
    }    
    if (pipe_exists && p1 > 0) {                           // Wait for stage 1 (if a pipe is used)
        if (waitpid(p1, &status, 0) < 0) {
            write(2, "waitpid stage1 failed\n", 22);
        }
    }
    return status;                                         // Return last child’s exit status
}

//------------------------------------------------------------------------------------------------------------------------------------

int get_job(struct Job *job) {
    initialize_job(job);

    int status = get_command(&job->pipeline[0]);
    if (status == 1) return 1;                        // "exit"
    if (job->pipeline[0].argc == 0) return 0;         // blank or error

    if (parse_pipeline(job) < 0) return 0;

    if (parse_input_redirection(job)  < 0) return 0;  // strips "< file" and sets infile_path
    if (parse_output_redirection(job) < 0) return 0;  // strips "> file" and sets outfile_path

    handle_background(job);
    return 2;
}

int run_job(struct Job *job) {
    struct FD fd_set;
    initialize_fd(job, &fd_set);
    if (setup_redirection(job, &fd_set) < 0) {     // open infile/outfile if requested
        return -1;
    }     
    if (fd_set.pipe_exists) {                      // create pipe if we have two stages
        if (pipe(fd_set.pipefd) < 0) {             // pipe failed then close any open files
            close_fd(&fd_set);
            write(2, "pipe failed\n", 12);
            return -1;
        }
    }    
    pid_t p0 = launch_stage0(job, &fd_set);         // Stage 0 stdio 
    if (p0 < 0)
        return -1;

    pid_t p1;                                       // Stage 1 stdio
    if (fd_set.pipe_exists)
        p1 = launch_stage1(job, &fd_set);

    if (job->background) return 0;                  // Background? don't wait 
    int status = wait_for_foreground(p0, p1, fd_set.pipe_exists);
    return 0;
}