#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 16     /* TO DO */
#define MAX_CH 256     /* TO DO */
#define MAX_PIPELINE_LEN 2

struct Command
{
  char *argv[MAX_ARGS+1]; // argument vector - array of argument strings
  unsigned int argc;      // argument count - number of arguments
  int background;          // '&' flag : background = 1 (no wait-time prompt user instantly) 
};

struct Job {
    struct Command pipeline[MAX_PIPELINE_LEN];
    unsigned int num_stages;   // 1 for now
    char *outfile_path;        // NULL if not specified
    char *infile_path;         // NULL if not specified
    int background;            // 0 = foreground, 1 = background
};

/* init helpers */
void initialize_command(struct Command *command);
void initialize_job(struct Job *job);
void handle_background(struct Job *job);


/* adapters */
int get_job(struct Job *job);  // 1=exit, 0=blank/error, 2=ok
int run_job(struct Job *job);  // returns 0 or -1

#endif
