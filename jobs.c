#include "myfunctions.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mystring.h"
#include "myheap.h"
#include "jobs.h"
#include <fcntl.h>
#include <sys/stat.h>

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

static int find_token(struct Command *command, const char *token) {
    for (int i = 0; i < command->argc; i++) {
        if (mystrcmp(command->argv[i], token) == 0) 
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

    if (job->infile_path) {                         // Handle input redirection
        *in_fd = open(job->infile_path, O_RDONLY);
        if (*in_fd < 0) {
            write(2, "open(<) failed\n", 15);
            return -1;
        }
    }    
    if (job->outfile_path) {                        // Handle output redirection
        *out_fd = open(job->outfile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (*out_fd < 0) {
            if (*in_fd >= 0) close(*in_fd);
            write(2, "open(>) failed\n", 15);
            return -1;
        }
    }
    return 0;                                       // success
}

int parse_pipeline(struct Job *job) {
    struct Command *command0 = &job->pipeline[0];
    int bar = find_token(command0, "|");
    if (bar < 0) {                                                          // no pipe
        job->num_stages = 1;
        return 0;
    }
    if (bar == 0 || bar == command0->argc - 1) {                       //first or last token is bar 
        write(2, "syntax error near '|'\n", 22);
        return -1;
    }
    
    initialize_command(&job->pipeline[1]);                                  // initialize the later part of the pipe (after |)
    struct Command *command1 = &job->pipeline[1];    
    for (int i = bar + 1; i < command0->argc; i++) {                   // move tokens after '|' into stage-1 
        command1->argv[command1->argc++] = command0->argv[i];
        command0->argv[i] = NULL;
    }    
    command0->argv[bar] = NULL;                                             // terminate stage-0 at '|'
    command0->argc = bar;

    job->num_stages = 2;    
    if (find_token(command1, "|") >= 0 || find_token(command0, "|") >= 0) { // error more than one pipe
        write(2, "only one '|' operator supported\n", 32);
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

    int last = 0;
    if (job->num_stages == 2) {
        last = 1;
    }
    struct Command *cmd = &job->pipeline[last];
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

    int status = get_command(&job->pipeline[0]);
    if (status == 1) return 1;                // "exit"
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
    int s0_in  = -1;
    int s0_out = -1;

    if (in_fd >= 0) {
        s0_in = in_fd;
    }

    if (use_pipe) {
        s0_out = pipefd[1];          // write end to stage 1
    } else if (job->outfile_path) {
        s0_out = out_fd;             // single stage with '>'
    }

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
    int s1_in  = -1;
    int s1_out = -1;

    /* the read end of the pipe always feeds stage 1 */
    s1_in = pipefd[0];

    /* use output redirection if present */
    if (job->outfile_path) {
        s1_out = out_fd;
    }

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

    int status = wait_for_foreground(p0, p1,use_pipe);

    return 0;
}

