#ifndef JOBS_H
#define JOBS_H
#include <sys/types.h>

#define MAX_ARGS 16     /* TO DO */
#define MAX_CH 256     /* TO DO */
#define MAX_PIPELINE_LEN 2

struct Command
{
  char *argv[MAX_ARGS+1]; // argument vector - array of argument strings
  unsigned int argc;      // argument count - number of arguments   
};

struct Job {
    struct Command pipeline[MAX_PIPELINE_LEN];
    unsigned int num_stages;   // 1 for now
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

/* init helpers */
void initialize_command(struct Command *command);
void initialize_job(struct Job *job);

void handle_background(struct Job *job);

int parse_pipeline(struct Job *job);
int parse_input_redirection(struct Job *job);
int parse_output_redirection(struct Job *job);

int setup_redirection(struct Job *job, struct FD *fd_set);


int wait_for_foreground(pid_t p0, pid_t p1, int pipe_exists);



static int find_token(struct Command *command, const char *token);
static void remove_tokens(struct Command *cmd, int pos, int count);

int parse_pipeline(struct Job *job);

void close_fd(struct FD *fd_set);
pid_t launch_stage0(struct Job *job, struct FD *fd_set);
pid_t launch_stage1(struct Job *job, struct FD *fd_set);
void initialize_fd(struct Job *job, struct FD *fd_set);


/* adapters */
int get_job(struct Job *job);  // 1=exit, 0=blank/error, 2=ok
int run_job(struct Job *job);  // returns 0 or -1

#endif
