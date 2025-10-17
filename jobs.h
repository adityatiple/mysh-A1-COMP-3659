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

/* init helpers */
void initialize_command(struct Command *command);
void initialize_job(struct Job *job);

void handle_background(struct Job *job);

int parse_pipeline(struct Job *job);
int parse_input_redirection(struct Job *job);
int parse_output_redirection(struct Job *job);

int setup_redirection(struct Job *job, int *in_fd, int *out_fd);


int wait_for_foreground(pid_t p0, pid_t p1, int use_pipe);



static int find_token(struct Command *command, const char *token);
static void remove_tokens(struct Command *cmd, int pos, int count);

int parse_pipeline(struct Job *job);

void close_fd(int *in_fd, int *out_fd);



/* adapters */
int get_job(struct Job *job);  // 1=exit, 0=blank/error, 2=ok
int run_job(struct Job *job);  // returns 0 or -1

#endif
