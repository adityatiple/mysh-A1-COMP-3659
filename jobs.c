#include "myfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mystring.h"
#include "myheap.h"
#include "jobs.h"

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

/* modifies job->background and strips '&' */
void handle_background(struct Job *job) {
    struct Command *cmd = &job->pipeline[0];
    if (cmd->argc > 0 && cmd->argv[cmd->argc - 1] &&
        mystrcmp(cmd->argv[cmd->argc - 1], "&") == 0) {
        cmd->argv[cmd->argc - 1] = NULL;
        cmd->argc--;
        job->background = 1;
    } else {
        job->background = 0;
    }
    cmd->argv[cmd->argc] = NULL;
}

int get_job(struct Job *job) {
    initialize_job(job);

    int rc = get_command(&job->pipeline[0]);
    if (rc == 1) return 1;                // "exit"
    if (job->pipeline[0].argc == 0) return 0;  // blank or error

    handle_background(job);
    if (parse_input_redirection(job)  < 0) return 0;  // strips "< file" and sets infile_path
    if (parse_output_redirection(job) < 0) return 0;  // strips "> file" and sets outfile_path
    job->num_stages = 1;
    return 2;
}

int run_job(struct Job *job) {
    if (job->num_stages != 1) {
        write(2, "unsupported pipeline length\n", 28);
        return -1;
    }

    pid_t pid = run_command(&job->pipeline[0]);

    if (pid == 0) {
        // nothing to run
        return 0;
    } 
    else if (pid < 0) {
        // error from fork/exec
        return -1;
    }

    if (job->background) {
        // run in background, don't wait
        return 0;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        write(2, "waitpid failed\n", 15);
        return -1;
    }

    return 0;
}

