#include "myfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mystring.h"
#include "myheap.h"
#include "jobs.h"
#include <fcntl.h>

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

static int find_token(struct Command *cmd, const char *tok) {
    for (int i = 0; i < cmd->argc; i++) {
        if (cmd->argv[i] && mystrcmp(cmd->argv[i], tok) == 0) 
        return i;
    }
    return -1;
}

static void remove_tokens(struct Command *cmd, int pos, int count) {
    int write = pos;
    for (int read = pos + count; read < (int)cmd->argc; read++, write++) {
        cmd->argv[write] = cmd->argv[read];
    }
    for (int k = write; k < (int)cmd->argc; k++) cmd->argv[k] = NULL;
    cmd->argc -= count;
}

int setup_redirection(struct Job *job, int *in_fd, int *out_fd) {
    *in_fd = -1;
    *out_fd = -1;

    // Handle input redirection
    if (job->infile_path) {
        *in_fd = open(job->infile_path, O_RDONLY);
        if (*in_fd < 0) {
            write(2, "open(<) failed\n", 15);
            return -1;
        }
    }

    // Handle output redirection
    if (job->outfile_path) {
        *out_fd = open(job->outfile_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*out_fd < 0) {
            if (*in_fd >= 0) close(*in_fd);
            write(2, "open(>) failed\n", 15);
            return -1;
        }
    }

    return 0;  // success
}

int parse_pipeline(struct Job *job) {
    struct Command *c0 = &job->pipeline[0];
    int bar = find_token(c0, "|");
    if (bar < 0) {                      // no pipe
        job->num_stages = 1;
        return 0;
    }
    if (bar == 0 || bar == (int)c0->argc - 1) {
        write(2, "syntax error near '|'\n", 22);
        return -1;
    }

    // init stage-1
    initialize_command(&job->pipeline[1]);

    struct Command *c1 = &job->pipeline[1];
    // move tokens after '|' into stage-1 (pointer move; same arena)
    for (int i = bar + 1; i < (int)c0->argc; i++) {
        c1->argv[c1->argc++] = c0->argv[i];
        c0->argv[i] = NULL;
    }

    // terminate stage-0 at '|'
    c0->argv[bar] = NULL;
    c0->argc = bar;

    job->num_stages = 2;

    // guard: reject more than one pipe
    if (find_token(c1, "|") >= 0 || find_token(c0, "|") >= 0) {
        write(2, "pipeline length > 2 not supported\n", 34);
        return -1;
    }
    return 0;
}

int parse_input_redirection(struct Job *job) {
    struct Command *c = &job->pipeline[0];
    int i = find_token(c, "<");
    if (i < 0) return 0;

    if (i == (int)c->argc - 1) {
        write(2, "missing filename after '<'\n", 27);
        return -1;
    }
    if (job->infile_path != NULL) {
        write(2, "multiple input redirections\n", 28);
        return -1;
    }
    job->infile_path = c->argv[i + 1];
    remove_tokens(c, i, 2);
    return 0;
}

int parse_output_redirection(struct Job *job) {
    unsigned last = (job->num_stages == 2) ? 1u : 0u;
    struct Command *c = &job->pipeline[last];

    int i = find_token(c, ">");
    if (i < 0) return 0;

    if (i == (int)c->argc - 1) {
        write(2, "missing filename after '>'\n", 27);
        return -1;
    }
    if (job->outfile_path != NULL) {
        write(2, "multiple output redirections\n", 29);
        return -1;
    }
    job->outfile_path = c->argv[i + 1];
    remove_tokens(c, i, 2);
    return 0;
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

int wait_for_foreground(pid_t p0, pid_t p1, int use_pipe) {
    int status = 0;

    /* Wait for stage 0 */
    if (p0 > 0) {
        if (waitpid(p0, &status, 0) < 0) {
            write(2, "waitpid stage0 failed\n", 22);
        }
    }

    /* Wait for stage 1 (if a pipe is used) */
    if (use_pipe && p1 > 0) {
        if (waitpid(p1, &status, 0) < 0) {
            write(2, "waitpid stage1 failed\n", 22);
        }
    }

    return status; // Return last child’s exit status
}

int get_job(struct Job *job) {
    initialize_job(job);

    int rc = get_command(&job->pipeline[0]);
    if (rc == 1) return 1;                // "exit"
    if (job->pipeline[0].argc == 0) return 0;  // blank or error

    if (parse_pipeline(job) < 0) return 0;

    
    if (parse_input_redirection(job)  < 0) return 0;  // strips "< file" and sets infile_path
    if (parse_output_redirection(job) < 0) return 0;  // strips "> file" and sets outfile_path

    handle_background(job);
    //job->num_stages = 1;
    return 2;
}

int run_job(struct Job *job) {
    int in_fd  = -1;
    int out_fd = -1;
    int pipefd[2] = {-1, -1};
    int use_pipe = (job->num_stages == 2);

    if (setup_redirection(job, &in_fd, &out_fd) < 0) {
        return -1;
    }

    /* create pipe if we have two stages */
    if (use_pipe) {
        if (pipe(pipefd) < 0) {
            if (in_fd  >= 0) close(in_fd);
            if (out_fd >= 0) close(out_fd);
            write(2, "pipe failed\n", 12);
            return -1;
        }
    }

    /* Stage 0 stdio */
    int s0_in  = (in_fd >= 0) ? in_fd : -1;
    int s0_out = use_pipe
                 ? pipefd[1]                   /* to stage 1 */
                 : (job->outfile_path ? out_fd /* single-stage with > */
                                      : -1);

    pid_t p0 = run_command(&job->pipeline[0], s0_in, s0_out);

    if (p0 <= 0) {
        if (in_fd  >= 0) close(in_fd);
        if (out_fd >= 0) close(out_fd);
        if (use_pipe) { close(pipefd[0]); close(pipefd[1]); }
        return -1;
    }

    /* Parent no longer needs stage 0's write end or input fd */
    if (s0_in  >= 0) close(s0_in);
    if (use_pipe && pipefd[1] >= 0) close(pipefd[1]);

    pid_t p1 = 0;

    if (use_pipe) {
        /* Stage 1 stdio */
        int s1_in  = pipefd[0];
        int s1_out = job->outfile_path ? out_fd : -1;

        p1 = run_command(&job->pipeline[1], s1_in, s1_out);

        /* Parent no longer needs stage 1's read end */
        if (s1_in >= 0) close(s1_in);
    }

    /* Parent no longer needs out_fd either */
    if (out_fd >= 0) close(out_fd);

    /* Background? don't wait */
    if (job->background) {
        return 0;
    }

    int status = wait_for_foreground(p0, p1, fds.use_pipe);

    return 0;
}
/*
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
*/

