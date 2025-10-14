#include "jobs.h"
#include <unistd.h>
#include "myfunctions.h"

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

int get_job(struct Job *job) {
    initialize_job(job);

    int rc = get_command(&job->pipeline[0]);
    if (rc == 1) return 1; // exit
    if (job->pipeline[0].argc == 0) return 0; // blank line

    job->num_stages = 1;  // only this one line is really needed
    return 2;             // success
}

int run_job(struct Job *job) {
    if (job->num_stages != 1) {
        write(2, "unsupported pipeline length\n", 28);
        return -1;
    }
    return run_command(&job->pipeline[0]);
}

